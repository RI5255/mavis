void putchar(char c);

void puts(const char *s) {
    while(*s) {
        putchar(*s++);
    }
    putchar('\n');
}