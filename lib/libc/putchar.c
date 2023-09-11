#ifdef __WASM_LIBC__
    #include <kernel/env.h>
#else
    #include <kernel/arch.h>
#endif

void putchar(char c) {
    arch_serial_write(c);
	return;
}