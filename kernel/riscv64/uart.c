#include "uart.h"

static inline void mmio_write8_paddr(uint64_t paddr, uint8_t val) {
    __asm__ __volatile__("sb %0, (%1)" ::"r"(val), "r"(paddr) : "memory");
}

static inline uint8_t mmio_read8_paddr(uint64_t paddr) {
    uint8_t val;
    __asm__ __volatile__("lb %0, (%1)" : "=r"(val) : "r"(paddr));
    return val;
}

void arch_serial_write(char ch) {
    while ((mmio_read8_paddr(UART_LSR) & UART_LSR_TX_FULL) == 0);
    mmio_write8_paddr(UART_THR, ch);
}

int arch_serial_read(void) {
    if ((mmio_read8_paddr(UART_LSR) & UART_LSR_RX_READY) == 0) {
        return -1;
    }

    return mmio_read8_paddr(UART_RBR);
}