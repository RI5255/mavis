#include <arch_types.h>
#include <kernel/task.h>
#include <kernel/memory.h>
#include <stdint.h>

// Simply save and restore the callee-saved register
__attribute__((naked)) 
void arch_context_switch(uint64_t *prev_sp, uint64_t *next_sp) {
     __asm__ __volatile__(
        "push rbp\n"
        "push rbx\n"
        "push r12\n"
        "push r13\n"
        "push r14\n"
        "push r15\n"
        "mov [rdi], rsp\n"
        "mov rsp, [rsi]\n"
        "pop r15\n"
        "pop r14\n"
        "pop r13\n"
        "pop r12\n"
        "pop rbx\n"
        "pop rbp\n"
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
        "pop rdi\n"
        "pop rsi\n"
        "jmp launch_vm_task\n"

        "l:\n"
        "jmp l\n"
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

    *--sp = (uint64_t)arch_vm_entry;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;

    task->arch = (struct arch_task) {
        .sp             = (uint64_t)sp,
        .stack_bottom   = (uint64_t)stack_bottom,
        .stack_top      = (uint64_t)stack_top
    };
}