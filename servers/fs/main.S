// todo: impl file system

    .section .rodata
    .global __fs_start
__fs_start:
     .incbin __wasm_path__
__fs_end:
    .globl __fs_size
__fs_size:
    .int __fs_end - __fs_start