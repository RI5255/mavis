#include <stdint.h>

/*
boot.c defines the boot function, which is the entry point of the kernel.
__stack_top is defined in the linker script of kernel. see kernel/kernel.ld
*/

extern uint8_t __stack_top[];

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        // use physical address only
        "csrw satp, zero\n"
        // prepare kernel stack
        "mv sp, %[stack_top]\n"
        // jump to kernel_main
        "j arch_boot\n"
        :
        : [stack_top] "r" (__stack_top)
    );
}

// architecture-dependent boot process
void arch_boot(void) {
    // set up trap handlers
    arch_set_trap_handlers();

    // jump to kernel_main
    __asm__ __volatile__ ("j kernel_main");
}