#pragma once

#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *buf, char c, size_t n);
int strcmp(const char *s1, const char *s2);