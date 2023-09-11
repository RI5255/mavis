#ifdef __WASM_LIBC__
    #include <kernel/env.h>
#else
    #include <kernel/arch.h>
#endif

char getchar(void) {
    // wait for input
    int c;
    while(1) {
        c = arch_serial_read();
        if(c != -1) break;
    }

    return c;
}