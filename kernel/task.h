#pragma once

#include <stdint.h>
#include <arch_types.h>
#include "list.h"
#include "memory.h"
#include "buffer.h"
#include "vm.h"
#include "message.h"

#define NUM_TASK_MAX    8
#define TASK_UNUSED     0
#define TASK_RUNNABLE   1
#define TASK_BLOCKED    2
#define TASK_NAME_LEN   16

typedef int         tid_t;

struct task {
    tid_t               tid;
    int                 state;
    char                name[TASK_NAME_LEN];

    struct arch_task    arch;
    struct malloc_pool  malloc_pool;
    
    void                *page_top;

    list_elem_t         next;

    struct message_box  message_box;
};

void task_init(void);
int task_create(const char *name, uint32_t ip, uint32_t *arg);
int vm_create(const char *name, void *image, int size);
void task_switch(void);
void task_resume(struct task *task);
void task_block(struct task *task);
void task_exit(int32_t code);