#pragma once

#define MIE_MTIE        (1 << 7)
#define MSTATUS_MIE     (1 << 3)

#define TIMER_INTERRUPT ((1lu << 31) | 7)
#define REG_MTIME       0x200BFF8U
#define REG_MTIMECMP    0x2004000U
#define INTERVAL        10000000

#define READ_CSR(reg)                                                           \
    ({                                                                          \
        unsigned long __tmp;                                                    \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                   \
        __tmp;                                                                  \
    })

#define WRITE_CSR(reg, value)                                                   \
        __asm__ __volatile__("csrw " #reg ", %0" :: "r"(value));                \

#define CSRRS(reg, mask)                                                        \
        __asm__ __volatile__("csrrs zero, " #reg ", %0" :: "r"(mask))
