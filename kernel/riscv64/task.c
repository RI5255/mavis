#include "asm.h"

#include <arch_types.h>
#include <kernel/task.h>
#include <kernel/memory.h>
#include <stdint.h>

__attribute__((naked)) 
void arch_context_switch(uint64_t *prev_sp, uint64_t *next_sp) {
     __asm__ __volatile__(
        "addi sp, sp, -13 * 8\n"
        "sd ra,  0  * 8(sp)\n"
        "sd s0,  1  * 8(sp)\n"
        "sd s1,  2  * 8(sp)\n"
        "sd s2,  3  * 8(sp)\n"
        "sd s3,  4  * 8(sp)\n"
        "sd s4,  5  * 8(sp)\n"
        "sd s5,  6  * 8(sp)\n"
        "sd s6,  7  * 8(sp)\n"
        "sd s7,  8  * 8(sp)\n"
        "sd s8,  9  * 8(sp)\n"
        "sd s9,  10 * 8(sp)\n"
        "sd s10, 11 * 8(sp)\n"
        "sd s11, 12 * 8(sp)\n"
        "sd sp, (a0)\n"
        "ld sp, (a1)\n"
        "ld ra,  0  * 8(sp)\n"
        "ld s0,  1  * 8(sp)\n"
        "ld s1,  2  * 8(sp)\n"
        "ld s2,  3  * 8(sp)\n"
        "ld s3,  4  * 8(sp)\n"
        "ld s4,  5  * 8(sp)\n"
        "ld s5,  6  * 8(sp)\n"
        "ld s6,  7  * 8(sp)\n"
        "ld s7,  8  * 8(sp)\n"
        "ld s8,  9  * 8(sp)\n"
        "ld s9,  10 * 8(sp)\n"
        "ld s10, 11 * 8(sp)\n"
        "ld s11, 12 * 8(sp)\n"
        "addi sp, sp, 13 * 8\n"
        "ret\n"
    );
}

void arch_task_switch(struct task *prev, struct task *next) {
    arch_context_switch(&prev->arch.sp, &next->arch.sp);
}

// launch_vm_task is defined in kernel/task.c
__attribute__((naked))
static void arch_vm_entry(void) {
    __asm__ __volatile__(        
        "ld a0, 0 * 8(sp)\n"
        "ld a1, 1 * 8(sp)\n"
        "add sp, sp, 2 * 8\n"
        "j launch_vm_task\n"

        "1:\n"
        "j 1b\n"
    );
}

// allocate kernel stack only.
void arch_init_idle_task(struct task *task) {
    uint64_t stack_bottom = (uint64_t)palloc(1);
    uint64_t stack_top = stack_bottom + PAGE_SIZE;

    uint64_t *sp = (uint64_t *)stack_top;

    task->arch = (struct arch_task) {
        .sp             = (uint64_t)sp,
        .stack_bottom   = (uint64_t)stack_bottom,
        .stack_top      = (uint64_t)stack_top
    };
}

void arch_vm_init(struct task *task, void *image, int size) {
    // todo: define paddr_t
    uint64_t stack_bottom = (uint64_t)palloc(1);
    uint64_t stack_top = stack_bottom + PAGE_SIZE;

    uint64_t *sp = (uint64_t *)stack_top;
    
    *--sp = size;
    *--sp = (uint64_t)image;

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
    *--sp = (uint64_t)arch_vm_entry;// ra

    task->arch = (struct arch_task) {
        .sp             = (uint64_t)sp,
        .stack_bottom   = (uint64_t)stack_bottom,
        .stack_top      = (uint64_t)stack_top
    };
}