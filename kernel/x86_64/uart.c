#include "asm.h"
#include "uart.h"

void arch_serial_write(char ch) {
    while ((mmio_read8_paddr(UART_LSR) & UART_LSR_TX_READY) == 0);
    mmio_write8_paddr(UART_THR, ch);
}

int arch_serial_read(void) {
    if ((mmio_read8_paddr(UART_LSR) & UART_LSR_RX_READY) == 0) {
        return -1;
    }

    return mmio_read8_paddr(UART_RBR);
}