#pragma once

#include <stdint.h>

struct arch_task{
    uint64_t stack_bottom;
    uint64_t sp;
    uint64_t stack_top;
};