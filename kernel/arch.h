#pragma once

#include <stdint.h>
struct task;

void arch_serial_write(char ch);
int arch_serial_read(void);

void arch_idle(void);
void arch_task_switch(struct task *prev, struct task *next);
void arch_vm_init(struct task *task, void *image, int size);
void arch_init_idle_task(struct task *task);