#include <kernel/message.h>
#include <kernel/env.h>
#include <string.h>
#include <lib/common/print.h>

extern char __hello_start[];
extern int __hello_size[];

#define CLOSE           0
#define OPEN            1
#define NUM_MAX_FILE    3

struct fd {
    int state;
};

static struct fd fds[NUM_MAX_FILE] = {};

static int alloc_fd(void) {
    for(int i = 0; i < NUM_MAX_FILE; i++) {
        if(fds[i].state == CLOSE) {
            fds[i].state = OPEN;
            return i;
        }
    }

    return -1;
}

int main(void) {
    struct message msg;
    for(;;) {
        ipc_receive(0, &msg);
        char *src = msg.src;
        
        switch(msg.type) {
            case OPEN_FILE_MSG:
                msg.type = OPEN_FILE_REPLY_MSG;
                // todo: fix this
                if(strcmp(msg.open_file.name, "hello") == 0) {
                    int fd = alloc_fd();

                    if(fd < 0)
                        WARN("fs", "alloc_fd failed");
                    
                    msg.open_file.fd = fd;
                    msg.open_file.size = __hello_size[0];
                } else {
                    WARN("fs", "%s not found", msg.open_file.name);
                    msg.open_file.fd = -1;
                }
                ipc_send(src, &msg);
                break;
            
            case GET_FILE_DATA_MSG: {
                msg.type = GET_FILE_DATA_REPLY_MSG;
                
                // todo: err handling
                // todo: fix this
                struct fd *fd = &fds[msg.get_file_data.fd];

                if(fd->state == OPEN) {
                    ipc_copy(src, __hello_start);
                } else {
                    WARN("fs", "%d is not open", msg.get_file_data.fd);
                }

                ipc_send(src, &msg);
                break;
            }

            case CLOSE_FILE_MSG: {
                msg.type = CLOSE_FILE_REPLY_MSG;

                struct fd *fd = &fds[msg.get_file_data.fd];
                if(fd->state == OPEN) {
                    fd->state  = CLOSE;
                } else {
                    WARN("fs", "%d is not open", msg.close_file.fd);
                }

                ipc_send(src, &msg);
                break;
            }
            
        }
    }
}