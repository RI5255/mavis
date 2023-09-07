#include "ipc.h"
#include "message.h"
#include "task.h"
#include "common.h"

extern struct task *current_task;
extern struct task tasks[NUM_TASK_MAX];

// todo: fix this
int ipc_send(tid_t dst_tid, struct message *msg) {
    struct task *dst =  &tasks[dst_tid - 1];
    
    msg->src = current_task->tid;
    
    dst->message_box.has_message = true;
    dst->message_box.message = *msg;

    // wake up receiver
    task_resume(dst);
    task_switch();

    return 0;
}

int ipc_receive(tid_t src_tid, struct message *msg) {
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