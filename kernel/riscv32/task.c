#include "asm.h"

#include <arch_types.h>
#include <kernel/task.h>
#include <kernel/memory.h>
#include <stdint.h>

__attribute__((naked)) 
void arch_context_switch(uint32_t *prev_sp, uint32_t *next_sp) {
     __asm__ __volatile__(
        "addi sp, sp, -13 * 4\n"
        "sw ra,  0  * 4(sp)\n"
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"
        "sw sp, (a0)\n"
        "lw sp, (a1)\n"
        "lw ra,  0  * 4(sp)\n"
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n"
        "ret\n"
    );
}

void arch_task_switch(struct task *prev, struct task *next) {
    arch_context_switch(&prev->arch.sp, &next->arch.sp);
}

// launch_vm_task is defined in kernel/task.c
__attribute__((naked))
static void arch_vm_entry(void) {
    // enable interrupt
    CSRRS(mstatus, MSTATUS_MIE);

    __asm__ __volatile__(        
        "lw a0, 0 * 4(sp)\n"
        "lw a1, 1 * 4(sp)\n"
        "add sp, sp, 2 * 4\n"
        "j launch_vm_task\n"

        "1:\n"
        "j 1b\n"
    );
}

// allocate kernel stack only.
void arch_init_idle_task(struct task *task) {
    uint32_t stack_bottom = (uint32_t)pmalloc(1);
    uint32_t stack_top = stack_bottom + PAGE_SIZE;

    uint32_t *sp = (uint32_t *)stack_top;

    task->arch = (struct arch_task) {
        .sp             = (uint32_t)sp,
        .stack_bottom   = (uint32_t)stack_bottom,
        .stack_top      = (uint32_t)stack_top
    };
}

void arch_vm_init(struct task *task, void *image, int size) {
    // todo: define paddr_t
    uint32_t stack_bottom = (uint32_t)pmalloc(1);
    uint32_t stack_top = stack_bottom + PAGE_SIZE;

    uint32_t *sp = (uint32_t *)stack_top;
    
    *--sp = size;
    *--sp = (uint32_t)image;

    *--sp = 0;                      // s11
    *--sp = 0;                      // s10
    *--sp = 0;                      // s9
    *--sp = 0;                      // s8
    *--sp = 0;                      // s7
    *--sp = 0;                      // s6
    *--sp = 0;                      // s5
    *--sp = 0;                      // s4
    *--sp = 0;                      // s3
    *--sp = 0;                      // s2
    *--sp = 0;                      // s1
    *--sp = 0;                      // s0
    *--sp = (uint32_t)arch_vm_entry;// ra

    task->arch = (struct arch_task) {
        .sp             = (uint32_t)sp,
        .stack_bottom   = (uint32_t)stack_bottom,
        .stack_top      = (uint32_t)stack_top
    };
}