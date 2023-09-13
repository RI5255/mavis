#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib/common/print.h>
#include <kernel/env.h>
#include <kernel/message.h>

int main(void) {
    struct message msg;
    for(;;) {
        // receive from any server
        ipc_receive(0, &msg);

        switch(msg.type) {
            case SPAWN_TASK_MSG:
                if(strcmp(msg.spawn_task.name, "hello") == 0) {
                    
                    // open file 
                    msg.type = OPEN_FILE_MSG;
                    memcpy(msg.open_file.name, "hello", 16);

                    ipc_call("fs", &msg);
                    
                    int fd      = msg.open_file.fd;
                    int size    = msg.open_file.size;

                    // get file data
                    char *buf = malloc(size);
                    ipc_share_buffer(buf, size);

                    msg.type = READ_FILE_DATA_MSG;
                    msg.get_file_data.fd = fd;

                    ipc_call("fs", &msg);


                    // close file
                    msg.type = CLOSE_FILE_MSG;
                    msg.close_file.fd = fd;
                    ipc_call("fs", &msg);

                    // create hello world task
                    INFO("vm", "launching hello...");
                    int tid = vm_create("hello", buf, size);

                    msg.type = SPAWN_TASK_REPLY_MSG;
                    msg.spawn_task.tid = tid;

                    ipc_send("shell", &msg);
                }
                break;
            
            case EXIT_TASK_MSG: {
                int exit_task = msg.exit_task.tid;
                INFO("vm", "tid %d exited normally", exit_task);

                task_destroy(exit_task);
                break;
            }
            
        }
    }
}