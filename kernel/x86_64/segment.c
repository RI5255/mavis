#include "asm.h"
#include "segment.h"

// global descriptor table
// base and limit is ignored in 64bit mode
static struct segment_descriptor gdt[3] = {
    [0] = {0},
    
    [1] = {
        .type   = 10,
        .S      = 1,
        .DPL    = 0,
        .P      = 1,
        .L      = 1,
        .D      = 0
    },
    
    [2] = {
        .type   = 2,
        .S      = 1,
        .DPL    = 0,
        .P      = 1,
        .L      = 1,
        .D      = 0
    }
};

void init_segment(void) {
    load_gdt(sizeof(gdt) - 1, (uint64_t)&gdt[0]);
    set_cs(0x8);
}