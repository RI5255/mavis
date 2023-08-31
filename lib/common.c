#include "common.h"
#include "env.h"
#include <stdarg.h>

void putchar(char c) {
    arch_serial_write(c);
	return;
}

char getchar(void) {
    // wait for input
    int c;
    while(1) {
        c = arch_serial_read();
        if(c != -1) break;
    }

    return c;
}

void puts(const char *s) {
    while(*s) {
        putchar(*s++);
    }
    putchar('\n');
}

// todo: return number of bytes written
int printf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case '\0':
                case '%':
                    putchar('%');
                    break;
                case 's': {
                    const char *s = va_arg(vargs, const char *);
                    while (*s) {
                        putchar(*s);
                        s++;
                    }
                    break;
                }
                case 'd': {
                    int value = va_arg(vargs, int);
                    if (value < 0) {
                        putchar('-');
                        value = -value;
                    }

                    int divisor = 1;
                    while (value / divisor > 9)
                        divisor *= 10;

                    while (divisor > 0) {
                        putchar('0' + value / divisor);
                        value %= divisor;
                        divisor /= 10;
                    }

                    break;
                }
                case 'x': {
                    int value = va_arg(vargs, int);
                    for (int i = 7; i >= 0; i--) {
                        int nibble = (value >> (i * 4)) & 0xf;
                        putchar("0123456789abcdef"[nibble]);
                    }
                }
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }

    va_end(vargs);

    return 0;
}

int strcmp(const char *s1, const char *s2) {
    while(*s1 && *s2) {
        if(*s1 != *s2)
            break;
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

__attribute__((noreturn)) void exit(int32_t code) {
    task_exit(code);
}