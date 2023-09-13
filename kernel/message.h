#pragma once

#include <stdint.h>
#include <stdbool.h>

enum message_type {
    SPAWN_TASK_MSG,
    SPAWN_TASK_REPLY_MSG,
    OPEN_FILE_MSG,
    OPEN_FILE_REPLY_MSG,
    GET_FILE_DATA_MSG,
    GET_FILE_DATA_REPLY_MSG,
    CLOSE_FILE_MSG,
    CLOSE_FILE_REPLY_MSG,
    EXIT_TASK_MSG,
    DESTROY_TASK_MSG
};

struct message {
    int     type;
    char    src[16];
    
    // todo: fix this
    union {
        struct {
            char    name[16];
            int     fd;
            int     size;
        } open_file;

        struct {
            int     fd;
        } get_file_data;

        struct {
            int     fd;
        } close_file;

        struct {
            char    name[16];
            int     tid;
        } spawn_task;

        struct {
            int     tid;
        } exit_task;

        struct {
            int     tid;
        } destroy_task;
    };
};