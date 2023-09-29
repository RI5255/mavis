#include <stdint.h>

#include "segment.h"
#include "interrupt.h"
#include "uart.h"
#include "task.h"

extern uint8_t __stack_top[];

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        // prepare kernel stack
        "mov rsp, %[stack_top]\n"

        "call init_segment\n"
        "call init_idt\n"

        // jump to kernel_main
        "jmp kernel_main\n"
        :
        : [stack_top] "r" (__stack_top)
    );
}

void putchar(char ch) {
    arch_serial_write(ch);
}

void puts(const char *s) {
    while(*s) {
        putchar(*s++);
    }
    putchar('\n');
}

struct task *task_a;
struct task *task_b;

void task_a_entry(void) {
    puts("starting task A\n");
    while (1) {
        putchar('A');
        switch_context(&task_a->sp, &task_b->sp);

        for (int i = 0; i < 30000000; i++)
            __asm__ __volatile__("nop");
    }
}

void task_b_entry(void) {
    puts("starting task B\n");
    while (1) {
        putchar('B');
        switch_context(&task_b->sp, &task_a->sp);

        for (int i = 0; i < 30000000; i++)
            __asm__ __volatile__("nop");
    }
}

void kernel_main(void) {
    puts("Hello, Kernel!");

    task_a = create_task((uint64_t)task_a_entry);
    task_b = create_task((uint64_t)task_b_entry);
    task_a_entry();

    for(;;)
        __asm__ __volatile("hlt");
}