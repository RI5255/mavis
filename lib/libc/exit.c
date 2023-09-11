#ifdef __WASM_LIBC__
    #include <kernel/env.h>

__attribute__((noreturn)) void exit(int code) {
    task_exit(code);
    __builtin_unreachable();
}

#else
#endif