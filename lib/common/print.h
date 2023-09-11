#pragma once

int printf(const char *fmt, ...);
void arch_idle();

#define PRINT_NL "\r\n"

#define SGR_ERR   "\e[1;91m"  // red + bold
#define SGR_WARN  "\e[1;33m"  // yellow + bold
#define SGR_INFO  "\e[0;96m"  // cyan
#define SGR_RESET "\e[0m"     // reset

#define INFO(server, fmt, ...)                                                  \
    do {                                                                        \
        printf(SGR_INFO "[%s] " fmt SGR_RESET PRINT_NL, server,                 \
               ##__VA_ARGS__);                                                  \
    } while (0)

#define WARN(server, fmt, ...)                                                  \
    do {                                                                        \
        printf(SGR_WARN "[%s] WARN: " fmt SGR_RESET PRINT_NL,                   \
               server, ##__VA_ARGS__);                                          \
    } while (0)

#define WAR
#define PANIC(fmt, ...)                                                         \
    do {                                                                        \
        printf("PANIC %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);     \
        arch_idle();                                                            \
    }while(0)