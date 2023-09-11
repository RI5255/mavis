#include "ipc.h"
#include "task.h"
#include <string.h>
#include <lib/common/print.h>

extern struct task *current_task;
extern struct task tasks[NUM_TASK_MAX];

// todo: fix this
int ipc_send(const char *name, struct message *msg) {

    int tid = task_lookup(name);

    if(!tid) {
        // todo: return err code
        PANIC("%s not found", name);
    }

    struct task *dst =  &tasks[tid - 1];

    // todo: fix this
    if(dst->state != TASK_BLOCKED && dst->state != TASK_RUNNABLE) {
        PANIC("%s is not runnable", name);
    }

    memcpy(msg->src, current_task->name, TASK_NAME_LEN);

    dst->message_box.has_message = true;
    dst->message_box.message = *msg;

    // wake up receiver
    task_resume(dst);
    task_switch();

    return 0;
}

// todo: Verify the existence of the sender.
int ipc_receive(const char *name, struct message *msg) {
    if(current_task->message_box.has_message) {
        *msg = current_task->message_box.message;
        current_task->message_box.has_message = false;
        return 0;
    }

    // wait for message
    task_block(current_task);
    task_switch();

    // received message
    *msg = current_task->message_box.message;
    current_task->message_box.has_message = false;

    return 0;
}

int ipc_call(const char *name, struct message *msg) {
    ipc_send(name, msg);
    ipc_receive(name, msg);
    return 0;
}