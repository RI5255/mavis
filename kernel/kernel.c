#include "kernel.h"
#include "common.h"
#include "task.h"

extern char __bss[], __bss_end[];
extern char __shell_start[], __vm_start[];
extern int __shell_size[], __vm_size[];

extern struct task *current_task;

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);
    
    arch_set_trap_handlers();

    // create idle task(kernel_main itself)
    // this is the only task that is not WASM binary
    task_init();
    
    // create shell server(tid=2)
    vm_create("shell", __shell_start, __shell_size[0]);

    // create vm server(tid=3)
    vm_create("vm", __vm_start, __vm_size[0]);
    
    task_switch();
    
    PANIC("switched to idle task");
}