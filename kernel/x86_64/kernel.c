#include <stdint.h>

#include "segment.h"
#include "interrupt.h"
#include "uart.h"

extern uint8_t __stack_top[];

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        // prepare kernel stack
        "mov rsp, %[stack_top]\n"
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

void kernel_main(void) {
    puts("Hello, Kernel!");
    
    init_segment();
    init_idt();

    for(;;)
        __asm__ __volatile("hlt");
}