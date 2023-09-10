#include <stdio.h>
#include <string.h>
#include <env.h>
#include <kernel/message.h>

int main(void) {
     while (1) {
        printf("shell> ");
        char cmdline[128];
        for (int i = 0;; i++) {
            char ch = getchar();
            putchar(ch);
            if (i == sizeof(cmdline) - 1) {
                printf("command line too long\n");
                continue;
            } else if (ch == '\r') {
                printf("\n");
                cmdline[i] = '\0';
                break;
            } else {
                cmdline[i] = ch;
            }
        }
        
        if (strcmp(cmdline, "hello") == 0) {
            struct message msg = {
                .type = SPAWN_TASK_MSG,
                .spawn_task = {.name = "hello"}
            };

            // todo: err handling
            ipc_call("vm", &msg);
            int new_task = msg.spawn_task.tid;

            // wait for end
            for(;;) {
                ipc_receive("vm", &msg);
                if(msg.type == DESTROY_TASK_MSG && msg.destroy_task.tid == new_task)
                    break;
            }
        }

        else if (strcmp(cmdline, "exit") == 0)
            break;
        else
            printf("unknown command: %s\n", cmdline);
    }

    return 0;
}