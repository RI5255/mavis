#include "ipc.h"
#include "list.h"
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

    if(tid == current_task->tid) {
        PANIC("send to itself is not allowed");
    }

    struct task *dst =  &tasks[tid - 1];

    bool ready = (dst->state == TASK_BLOCKED
              && (dst->wait_for == IPC_ANY || dst->wait_for == current_task->tid));

    if(!ready) {
        WARN("kernel", "%s is not ready", dst->name);
        list_push_back(&dst->senders, &current_task->waitqueue_next);
        task_block(current_task);
        task_switch();
    }

    // copy message
    memcpy(msg->src, current_task->name, TASK_NAME_LEN);
    dst->message = *msg;

    // wake up receiver
    task_resume(dst);

    return 0;
}

/*
    ipc_receive takes a tid as an argument. 
    This is because the sender is known when ipc_receive is called. 
    Therefore, unlike ipc_send, the server name is not used.
*/
int ipc_receive(int src, struct message *msg) {
    LIST_FOR_EACH(sender, &current_task->senders, struct task, waitqueue_next) {
        if(src == IPC_ANY || sender->tid == src) {
            INFO("kernel", "waking up %s", sender->name);
            list_remove(&sender->waitqueue_next);
            task_resume(sender);
            src = sender->tid;
            break;
        }
    }

    // wait for message
    current_task->wait_for = src;
    task_block(current_task);
    task_switch();

    // receive message
    current_task->wait_for = IPC_DENY;
    *msg = current_task->message;

    return 0;
}

int ipc_call(const char *name, struct message *msg) {
    ipc_send(name, msg);
    
    int src = task_lookup(name);
    ipc_receive(src, msg);
    
    return 0;
}