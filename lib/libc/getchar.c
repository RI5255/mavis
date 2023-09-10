#include "env.h"

char getchar(void) {
    // wait for input
    int c;
    while(1) {
        c = arch_serial_read();
        if(c != -1) break;
    }

    return c;
}