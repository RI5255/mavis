#include "kernel.h"
#include "arch.h"
#include "task.h"
#include <string.h>
#include <lib/common/print.h>

extern char __bss[], __bss_end[];
extern char __shell_start[], __vm_start[], __fs_start[];
extern int __shell_size[], __vm_size[], __fs_size[];

extern struct task *current_task;

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);
    
    init_memory();
    
    // create idle task(kernel_main itself)
    // this is the only task that is not WASM binary
    task_init();

    // create vm server
    vm_create("vm", __vm_start, __vm_size[0]);

    // create shell server
    vm_create("shell", __shell_start, __shell_size[0]);

    // create fs server
    vm_create("fs", __fs_start, __fs_size[0]);
    
    task_switch();
    
    PANIC("switched to idle task");
}