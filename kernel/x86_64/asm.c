#include "asm.h"

void load_gdt(uint16_t limit, uint64_t offset) {
    __asm__ __volatile__ (
        "push rsi\n"
        "push di\n" 
        "lgdt [rsp]\n"
    );
}


void set_cs(uint16_t cs) {
    __asm__ __volatile__ (
        "lea rax, .L0[rip]\n"
        "push di\n"
        "push rax\n"
        "retfq\n"
        ".L0:\n"
    );
}

void load_idt(uint16_t limit, uint64_t offset) {
    __asm__ __volatile__ (
        "push rsi\n"
        "push di\n"
        "lidt [rsp]\n"
    );
}

void mmio_write8_paddr(uint16_t paddr, uint8_t val) {
    __asm__ __volatile__ (
        "mov dx, di\n"
        "mov al, sil\n"
        "out dx, al\n"
        :
        :
        : "rdx", "rax"
    );
}

uint8_t mmio_read8_paddr(uint16_t paddr) {
    __asm__ __volatile__ (
        "xor eax, eax\n"
        "mov dx, di\n"
        "in al, dx\n"
        :
        :
        : "rdx", "rax"
    );
}
