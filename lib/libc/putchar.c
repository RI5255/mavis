#include "env.h"

void putchar(char c) {
    arch_serial_write(c);
	return;
}