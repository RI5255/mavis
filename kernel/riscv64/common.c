void arch_idle(void) {
    for(;;)                                                                 \
        __asm__ __volatile__("wfi");
}