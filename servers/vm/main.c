#include <lib/env.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <kernel/message.h>

// todo: impl file system

extern char __hello_start[];
extern int __hello_size[];

int main(void) {
    struct message msg;
    for(;;) {
        // receive from any server
        ipc_receive(0, &msg);
        int src = msg.src;

        // todo: use switch-case (cannot use this since runtime has not support branch_table yet)
        if(msg.type == SPAWN_TASK_MSG) {
            if(strcmp(msg.spawn_task.name, "hello") == 0) {
                
                // create hello world task
                puts("[vm] launching hello...");
                int tid = vm_create(__hello_start, __hello_size[0]);

                msg = (struct message ) {
                    .type = SPAWN_TASK_REPLY_MSG,
                    .spawn_task = {.tid = tid}
                };

                ipc_send(src, &msg);
            }
        }

        if(msg.type == EXIT_TASK_MSG) {
            int exit_task = msg.exit_task.tid;

            printf("[vm] tid %d exited normally\n", exit_task);

            // send message to pager task
            // todo: fix this
            msg = (struct message) {
                .type = DESTROY_TASK_MSG,
                .destroy_task = {.tid = exit_task}
            };
            ipc_send(2, &msg);
        }
    }
}