// todo: impl file system

    .section .rodata
    .global __vm_start
__vm_start:
     .incbin __wasm_path__
__vm_end:
    .globl __vm_size
__vm_size:
    .int __vm_end - __vm_start