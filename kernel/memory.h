#pragma once

#include <stdint.h> 
#include <stddef.h>
#include "list.h"

#define PAGE_SIZE   4096

// clang extention
#define align_up(value, align) __builtin_align_up(value, align)
#define is_aligned(value, align) __builtin_is_aligned(value, align)

struct page {
    list_elem_t link;
    int ref_count;
};

struct memory_zone {
    size_t  num_pages;
    void    *base;
    struct  page pages[];
};

void *palloc(size_t num_pages);
void pfree(struct page *page);
void pfree_by_list(list_t *list);
void *malloc(size_t size);
void init_memory(void);