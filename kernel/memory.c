#include "memory.h"
#include "task.h"
#include "list.h"
#include <stdbool.h>
#include <string.h>
#include <lib/common/print.h>

extern uint8_t __pmalloc_pool_start[], __pmalloc_pool_end[];
extern struct task *current_task;

static struct memory_zone *zone;

static bool is_contiguously_free(size_t start, size_t num_pages) {
    for(size_t i = 0; i < num_pages; i++) {
        if (zone->pages[start + i].ref_count != 0) {
            return false;
        }
    }
    return true;
}

void *palloc(size_t num_pages) {
    for (size_t start = 0; start < zone->num_pages; start++) {
        void *paddr = zone->base + start * PAGE_SIZE;
        if (is_contiguously_free(start, num_pages)) {
            // 空いているので各物理ページを割り当てる
            for (size_t i = 0; i < num_pages; i++) {
                struct page *page = &zone->pages[start + i];
                page->ref_count = 1;
                list_push_back(&current_task->pages, &page->link);
            }
            
            memset(paddr, 0, PAGE_SIZE * num_pages);
            return paddr;
        }
    }
    WARN("kernel", "pm: run out of memory");
    return NULL;
}

void pfree(struct page *page) {
    if(--page->ref_count == 0);
        list_remove(&page->link);
}

void pfree_by_list(list_t *list) {
    LIST_FOR_EACH(page, list, struct page, link) {
        pfree(page);
    }
}

void *malloc(size_t size) {
    static uint8_t *next_paddr;

    uint8_t *heap = current_task->heap;

    // init heap
    if(!heap) {
        heap = palloc(1);
        current_task->heap = heap;
        next_paddr = heap;
    }

    if(next_paddr + size > heap + PAGE_SIZE) {
        // extend heap
        heap = palloc(1);
        current_task->heap = heap;
        next_paddr = heap;
    }

    void *paddr = next_paddr;
    next_paddr = align_up(paddr + size, 0x10);

    memset(paddr, 0, size);

    return paddr;
}

void init_memory(void) {
    zone = (struct memory_zone *)__pmalloc_pool_start;
    int num_pages = (__pmalloc_pool_end - __pmalloc_pool_start) / (sizeof(struct page) + PAGE_SIZE);

    uint8_t *header_end = align_up(&zone->pages[num_pages], PAGE_SIZE);
    int header_size = header_end - (uint8_t *)zone;

    num_pages -= header_size / PAGE_SIZE;

    // init memory zone
    zone->num_pages = num_pages;
    zone->base = header_end;
}