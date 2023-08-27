#pragma once

#include <stddef.h>
#include "arch.h"

#define PANIC(fmt, ...)                                                         \
    do {                                                                        \
        printf("PANIC %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);     \
        arch_idle();                                                            \
    }while(0)

#define PAGE_SIZE 4096

// clang extention
#define align_up(value, align) __builtin_align_up(value, align)
#define is_aligned(value, align) __builtin_is_aligned(value, align)

void *memset(void *buf, char c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
void putchar(char ch);
char getchar(void);
void puts(const char *s);
void printf(const char *fmt, ...);
