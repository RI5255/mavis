#pragma once

#include "asm.h"
#include "segment.h"
#include <stdint.h>

struct interrupt_descriptor {
    uint16_t    offset_low;
    struct segment_selector segment_selector;

    struct {
        uint8_t IST     : 3;
        uint8_t Rsvd    : 5;
        uint8_t type    : 4;
        uint8_t Rsvd    : 1;
        uint8_t DPL     : 2;
        uint8_t P       : 1; 
    }__attribute__((packed)) attributes;
    
    uint32_t    offset_high;
    uint32_t    Rsvd;
}__attribute__((packed)) ;

void init_idt(void);