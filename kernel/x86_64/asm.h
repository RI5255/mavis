#pragma once

#include <stdint.h>

#define MERGE(a,b)  a##b
#define LABEL(a) MERGE(_Rsvd, a)
#define Rsvd LABEL(__LINE__)

void load_gdt(uint16_t limit, uint64_t offset);
void set_cs(uint16_t cs);
void load_idt(uint16_t limit, uint64_t offset);