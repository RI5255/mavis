#include "interrupt.h"
#include "asm.h"

static struct interrupt_descriptor idt[256];

// todo: fix this
__attribute__((noreturn))
static void x86_64_default_interrupt_handler(void) {
    for(;;) {
        __asm__ __volatile__("hlt");
    }
}

static void set_idt_entry(int vec, uint64_t offset) {
    idt[vec] = (struct interrupt_descriptor) {
        .offset_low         = offset & 0xffffu,
        .segment_selector   = {.index = 1},
        .attributes         = {
                .IST    = 0,
                .type   = 14,
                .DPL    = 0,
                .P      = 1
        },
        .offset_high        = (offset >> 16) & 0xffffffffu,
    };
}

void init_idt(void) {
    for(int i = 0; i < 256; i++) {
        set_idt_entry(i, (uint64_t)x86_64_default_interrupt_handler);
    }

    load_idt(sizeof(idt) - 1, (uint64_t)&idt);
}