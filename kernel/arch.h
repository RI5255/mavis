#pragma once

#include <stdint.h>
struct task;

void arch_serial_write(char ch);
int arch_serial_read(void);

void arch_idle(void);
void arch_set_trap_handlers(void);
void arch_task_switch(struct task *prev, struct task *next);
void arch_task_init(struct task *task, uint32_t ip, void *arg);