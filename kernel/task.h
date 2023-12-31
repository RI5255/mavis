#pragma once

#include <stdint.h>
#include <arch_types.h>
#include "list.h"
#include "memory.h"
#include "message.h"

#define NUM_TASK_MAX    8
#define TASK_UNUSED     0
#define TASK_RUNNABLE   1
#define TASK_BLOCKED    2
#define TASK_NAME_LEN   16

typedef int         tid_t;

struct shared_buffer {
    int     len;
    char    *p;
};

struct task {
    tid_t                   tid;
    int                     state;
    char                    name[TASK_NAME_LEN];

    bool                    destroyed;

    struct arch_task        arch;

    list_t                  pages;
    void                    *heap;
    
    list_elem_t             next;
    list_elem_t             waitqueue_next;
    list_t                  senders;
    tid_t                   wait_for;
    struct shared_buffer    shared_buffer;
    struct message          message;
};

void task_init(void);
int vm_create(const char *name, void *image, int size);
void task_switch(void);
void task_resume(struct task *task);
void task_block(struct task *task);
int task_lookup(const char *name);
void task_exit(int32_t code);
int task_destroy(int tid);