#include "env.h"

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

// todo: impl printf