#include <stdint.h>

/*
    env.h defines a list of built-in functions that the runtime has. 
    These can be used from the WASM binary.  
*/

__attribute__((
    __import_module__("env"),
    __import_name__("arch_serial_write"),
))
void arch_serial_write(char ch);

__attribute__((
    __import_module__("env"),
    __import_name__("arch_serial_read"),
))
int arch_serial_read(void);

__attribute__((
    __import_module__("env"),
    __import_name__("task_exit"),
))
__attribute__((noreturn))
void task_exit(int32_t code);

__attribute__((
    __import_module__("env"),
    __import_name__("vm_create"),
))
int vm_create(void *image, int size);

__attribute__((
    __import_module__("env"),
    __import_name__("ipc_send"),
))
int ipc_send(int dst_tid, void *msg);

__attribute__((
    __import_module__("env"),
    __import_name__("ipc_receive"),
))
int ipc_receive(int src_tid, void *msg);

__attribute__((
    __import_module__("env"),
    __import_name__("ipc_call"),
))
int ipc_call(int dst_tid, void *msg);