#pragma once

#include <stdint.h>

#define UART_ADDR               (0x03f8)

// Transmitter Holding Register.
#define UART_THR                (UART_ADDR + 0x00)

// Receiver Buffer Register.
#define UART_RBR                (UART_ADDR + 0x00)

/// Interrupt Enable Register.
#define UART_IER                (UART_ADDR + 0x01)

/// FIFO Control Register.
#define UART_FCR                (UART_ADDR + 0x02)

/// Line Status Register.
#define UART_LSR                (UART_ADDR + 0x05)
#define UART_LSR_RX_READY       (1 << 0)
#define UART_LSR_TX_READY       (1 << 5)

// FIFO Control Register.
#define UART_FCR_FIFO_ENABLE    (1 << 0)
#define UART_FCR_FIFO_CLEAR     (0b11 << 1)

void arch_serial_write(char ch);
int arch_serial_read(void);