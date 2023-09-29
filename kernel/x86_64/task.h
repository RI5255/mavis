#pragma once

#include <stdint.h>

#define NUM_TASK_MAX    8
#define TASK_UNUSED     0
#define TASK_RUNNABLE   1

struct task {
    int tid;
    int state;
    uint64_t sp;
    uint8_t stack[8192];
};

void switch_context(uint64_t *prev_sp, uint64_t *next_sp);
struct task *create_task(uint64_t pc);