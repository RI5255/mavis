.intel_syntax noprefix 
.section .text

# void load_gdt(uint16_t limit, uint64_t offset)
.global load_gdt
load_gdt:
    push rbp
    mov rbp, rsp
    push rsi
    push di 
    lgdt [rsp]
    leave
    ret 

# ss, ed, ds, fs is not used in 64bit mode
# void set_cs(uint16_t cs)
.global set_cs
set_cs:
    push rbp
    mov rbp, rsp
    lea rax, .L0[rip]
    push di
    push rax
    retfq
.L0:
    leave 
    ret
