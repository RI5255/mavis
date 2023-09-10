#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE           4096
#define WASM_PAGE_SIZE      (16 * PAGE_SIZE)

#define align(val, align)   __builtin_align_up(val, align)

extern uint8_t __heap_base[], __heap_end[];

void *malloc(size_t size) {
    static uint8_t *next_ptr = __heap_base;

    if(next_ptr + size > __heap_end)
        return NULL;
    
    uint8_t *ptr = next_ptr;
    
    next_ptr = align(next_ptr + size, 0x10);

    return ptr;
}