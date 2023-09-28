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

# void load_idt(uint16_t limit, uint64_t offset)
.global load_idt
load_idt:
    push rbp
    mov rbp, rsp
    push rsi 
    push di 
    lidt [rsp]
    leave 
    ret 

# void mmio_write8_paddr(uint16_t paddr, uint8_t val)
.global mmio_write8_paddr
mmio_write8_paddr:
    mov dx, di
    mov al, sil
    out dx, al
    ret

# uint8_t mmio_read8_paddr(uint16_t paddr)
.global mmio_read8_paddr
mmio_read8_paddr:
    xor eax, eax
    mov dx, di
    in al, dx
    ret
