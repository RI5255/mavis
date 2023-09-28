#pragma once

#include <stdint.h>

struct segment_selector {
    uint8_t     RPL     : 2;
    uint8_t     TI      : 1;
    uint16_t    index   : 13;
} __attribute__ ((packed));

struct segment_descriptor {
    uint16_t    limit_low;
    uint16_t    base_low;
    uint8_t     base_mid;
    uint8_t     type            : 4;
    uint8_t     S               : 1;
    uint8_t     DPL             : 2;
    uint8_t     P               : 1;
    uint8_t     limit_high      : 4;
    uint8_t     AVL             : 1;
    uint8_t     L               : 1;
    uint8_t     D               : 1;
    uint8_t     G               : 1;
    uint8_t     base_high;
} __attribute__ ((packed));

void init_segment(void);