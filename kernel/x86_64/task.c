#include "task.h"

#include <stddef.h>

static struct task tasks[NUM_TASK_MAX];

// Simply save and restore the callee-saved register
__attribute__((naked)) 
void switch_context(uint64_t *prev_sp, uint64_t *next_sp) {
     __asm__ __volatile__(
        "push rbp\n"
        "push rbx\n"
        "push r12\n"
        "push r13\n"
        "push r14\n"
        "push r15\n"
        "mov [rdi], rsp\n"
        "mov rsp, [rsi]\n"
        "pop r15\n"
        "pop r14\n"
        "pop r13\n"
        "pop r12\n"
        "pop rbx\n"
        "pop rbp\n"
        "ret\n"
    );
}

struct task *create_task(uint64_t pc) {
    struct task *task = NULL;
    int i;
    for (i = 0; i < NUM_TASK_MAX; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            task = &tasks[i];
            break;
        }
    }

    // switch_context() で復帰できるように、スタックに呼び出し先保存レジスタを積む
    uint64_t *sp = (uint64_t *) &task->stack[sizeof(task->stack)];
    *--sp = (uint64_t)pc;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;

    // 各フィールドを初期化
    task->tid = i + 1;
    task->state = TASK_RUNNABLE;
    task->sp = (uint64_t) sp;
    return task;
}