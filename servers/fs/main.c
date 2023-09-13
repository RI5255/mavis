#include "fs.h"
#include <kernel/message.h>
#include <kernel/env.h>
#include <string.h>
#include <lib/common/print.h>

extern char __image[];

static struct file files[NUM_MAX_FILE] = {};

static int alloc_fd(void) {
    for(int i = 0; i < NUM_MAX_FILE; i++) {
        if(files[i].state == CLOSE) {
            files[i].state = OPEN;
            return i;
        }
    }
    return -1;
}

static struct fs_entry *find_file(const char *name) {
    struct fs_header *header = (struct fs_header *)__image;
    int num_files = header->num_files;

    struct fs_entry *entry = (struct fs_entry *)(__image + header->header_size);

    for(int i = 0; i < num_files; i++) {
        if(strcmp(name, entry->name) == 0)
            return entry;
        entry++;
    }

    return NULL;
}

int main(void) {
    struct message msg;
    for(;;) {
        ipc_receive(0, &msg);
        char *src = msg.src;
        
        switch(msg.type) {
            case OPEN_FILE_MSG:
                msg.type = OPEN_FILE_REPLY_MSG;
                
                struct fs_entry *entry = find_file(msg.open_file.name);
                int fd;

                if(!entry) {
                    WARN("fs", "%s not found", msg.open_file.name);
                    msg.open_file.fd = -1;
                } else {
                    msg.open_file.fd = alloc_fd();
                    msg.open_file.size = entry->len;
                }
                ipc_send(src, &msg);
                break;
            
            case GET_FILE_DATA_MSG: {
                msg.type = GET_FILE_DATA_REPLY_MSG;
                
                // todo: err handling
                // todo: fix this
                struct file *file = &files[msg.get_file_data.fd];
                if(file->state == OPEN) {
                    ipc_copy(src, __image + entry->offset);
                } else {
                    WARN("fs", "%d is not open", msg.get_file_data.fd);
                }

                ipc_send(src, &msg);
                break;
            }

            case CLOSE_FILE_MSG: {
                msg.type = CLOSE_FILE_REPLY_MSG;
                
                struct file *file = &files[msg.get_file_data.fd];
                if(file->state == OPEN) {
                    file->state  = CLOSE;
                } else {
                    WARN("fs", "%d is not open", msg.close_file.fd);
                }

                ipc_send(src, &msg);
                break;
            }
            
        }
    }
}