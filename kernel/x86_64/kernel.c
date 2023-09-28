#include <stdint.h>

#include "segment.h"
#include "interrupt.h"

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

void kernel_main(void) {
    init_segment();
    init_idt();

    for(;;)
        __asm__ __volatile("hlt");

}