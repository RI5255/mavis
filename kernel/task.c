#include "task.h"
#include "arch.h"
#include "buffer.h"
#include "memory.h"
#include "vm.h"
#include "ipc.h"
#include <lib/common/print.h>
#include <string.h>

struct task tasks[NUM_TASK_MAX];
static list_t  runqueue = (list_t) {
    .prev   = &runqueue,
    .next   = &runqueue
};

static int alloc_tid(void) {
    int i;
    for (i = 0; i < NUM_TASK_MAX; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            return i + 1;
        }
    }
    return 0;
}

static void vm_init(struct task *task, const char *name, int tid, void *image, int size) {
    // init malloc_pool
    // In the current implementation, the top of the page is used as the header.
    LIST_INIT(&task->malloc_pool.pages);
    struct page *page = pmalloc(1);
    task->page_top = page;
    list_push_back(&task->malloc_pool.pages, &page->link);
    task->malloc_pool.next_ptr = align_up(page->base, 0x10);
   
    // init stack
    arch_vm_init(task, image, size);

    // set task name
    memcpy(task->name, name, TASK_NAME_LEN);

    // set tid
    task->tid = tid;

    // init lists
    LIST_INIT(&task->senders);
    list_elem_init(&task->next);
    list_elem_init(&task->waitqueue_next);
}

void task_resume(struct task *task) {
    task->state = TASK_RUNNABLE;
    list_push_back(&runqueue, &task->next);
}

void task_block(struct task *task) {
    task->state = TASK_BLOCKED;
}

// todo: impl file system and fix this
// this is entry point of vm_task
void launch_vm_task(void *image, int size) {
    struct buffer *buf = newbuffer(image, size);
    module *m = new_module(buf);
    struct context *ctx = create_context(m);
    run_vm(ctx);
}

int vm_create(const char *name, void *image, int size) {
    // alloc tid
    int tid = alloc_tid();
    if(!tid) {
        // todo: return error code
        PANIC("no free task slots");
    }

    // init task struct
    struct task *task = &tasks[tid - 1];
    vm_init(task, name, tid, image, size);

    // push to runqueu
    task_resume(task);

    return tid;
}

struct task *current_task;

struct task *schedule(void) {
    struct task *next = LIST_POP_TAIL(&runqueue, struct task, next);

    if(next)
        return next;

    if(current_task->state == TASK_RUNNABLE)
        return current_task;

    __builtin_unreachable();
}

// create idle task
void task_init(void) {
    int tid = alloc_tid();

    struct task *idle_task = &tasks[tid - 1];
    arch_init_idle_task(idle_task);

    task_resume(idle_task);
    current_task = idle_task;
}

void task_switch(void) {
    struct task *prev = current_task;
    struct task *next = schedule();

    if(prev == next)
        return;
    
    //todo: push back to runqueue if not idle_task?

    current_task = next;
    arch_task_switch(prev, next);
}

int task_lookup(const char *name) {
    for(int i = 0; i < NUM_TASK_MAX; i++) {
        struct task *task = &tasks[i];
        if(strcmp(name, task->name) == 0)
            return task->tid;
    }
    return 0;
}

__attribute__((noreturn))
void task_exit(int32_t code) {
    // set state
    current_task->state = TASK_EXITED;

    // send message to vm task
    struct message msg = {
        .type = EXIT_TASK_MSG,
        .exit_task = {.tid = current_task->tid}
    };
    ipc_send("vm", &msg);

    task_switch();

    // never reach here
    PANIC("unreachable");
    __builtin_unreachable();
}

// todo: return err code
int task_destroy(int tid) {
    if(tid < 1 || NUM_TASK_MAX < tid) {
        PANIC("tid %d not found", tid);
    }

    struct task *task = &tasks[tid - 1];

    if(task->state != TASK_EXITED) {
        PANIC("tid %d is not exited yet", tid);
    }

    // free memory
    pfree(task->page_top);

    // set state
    task->state = TASK_UNUSED;

    // todo: init task struct?
    
    // send message to shell
    if(strcmp(task->name, "shell") != 0) {
        struct message msg = {
            .type = DESTROY_TASK_MSG,
            .destroy_task = {.tid = task->tid}
        };
        ipc_send("shell", &msg);
    }

    return 0;
}