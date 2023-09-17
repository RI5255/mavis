
#include "asm.h"

#include <stdint.h>
#include <lib/common/print.h>
#include <kernel/task.h>


typedef struct __attribute__((packed)) {
    uint32_t mepc;
    uint32_t mstatus;
    uint32_t ra;
    uint32_t sp;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
} trap_frame;

// interrupt entry
__attribute__((naked))
__attribute__((aligned(4)))
static void trap_entry(void) {
    __asm__ __volatile__(
        "csrw mscratch, a0\n"
        "mv a0, sp\n"
        "addi a0, a0, -4 * 33\n"
        "sw ra,  4 * 2(a0)\n"
        "sw sp,  4 * 3(a0)\n"
        "sw gp,  4 * 4(a0)\n"
        "sw tp,  4 * 5(a0)\n"
        "sw t0,  4 * 6(a0)\n"
        "sw t1,  4 * 7(a0)\n"
        "sw t2,  4 * 8(a0)\n"
        "sw t3,  4 * 9(a0)\n"
        "sw t4,  4 * 10(a0)\n"
        "sw t5,  4 * 11(a0)\n"
        "sw t6,  4 * 12(a0)\n"
        "sw a1,  4 * 14(a0)\n"
        "sw a2,  4 * 15(a0)\n"
        "sw a3,  4 * 16(a0)\n"
        "sw a4,  4 * 17(a0)\n"
        "sw a5,  4 * 18(a0)\n"
        "sw a6,  4 * 19(a0)\n"
        "sw a7,  4 * 20(a0)\n"
        "sw s0,  4 * 21(a0)\n"
        "sw s1,  4 * 22(a0)\n"
        "sw s2,  4 * 23(a0)\n"
        "sw s3,  4 * 24(a0)\n"
        "sw s4,  4 * 25(a0)\n"
        "sw s5,  4 * 26(a0)\n"
        "sw s6,  4 * 27(a0)\n"
        "sw s7,  4 * 28(a0)\n"
        "sw s8,  4 * 29(a0)\n"
        "sw s9,  4 * 30(a0)\n"
        "sw s10, 4 * 31(a0)\n"
        "sw s11, 4 * 32(a0)\n"

        "mv sp, a0\n"
        "csrr a0, mscratch\n"
        "sw a0,  4 * 13(sp)\n"
        
        "csrr  a0, mepc\n"
        "sw a0, 4 * 0(sp)\n"
        "csrr  a0, mstatus\n"
        "sw a0, 4 * 1(sp)\n"

        "mv a0, sp\n"
        "call riscv32_handle_trap\n"

        "lw a0, 4 * 0(sp)\n"
        "csrw mepc, a0\n"
        "lw a0, 4 * 1(sp)\n"
        "csrw mstatus, a0\n"

        "lw ra,  4 * 2(sp)\n"
        "lw gp,  4 * 4(sp)\n"
        "lw tp,  4 * 5(sp)\n"
        "lw t0,  4 * 6(sp)\n"
        "lw t1,  4 * 7(sp)\n"
        "lw t2,  4 * 8(sp)\n"
        "lw t3,  4 * 9(sp)\n"
        "lw t4,  4 * 10(sp)\n"
        "lw t5,  4 * 11(sp)\n"
        "lw t6,  4 * 12(sp)\n"
        "lw a0,  4 * 13(sp)\n"
        "lw a1,  4 * 14(sp)\n"
        "lw a2,  4 * 15(sp)\n"
        "lw a3,  4 * 16(sp)\n"
        "lw a4,  4 * 17(sp)\n"
        "lw a5,  4 * 18(sp)\n"
        "lw a6,  4 * 19(sp)\n"
        "lw a7,  4 * 20(sp)\n"
        "lw s0,  4 * 21(sp)\n"
        "lw s1,  4 * 22(sp)\n"
        "lw s2,  4 * 23(sp)\n"
        "lw s3,  4 * 24(sp)\n"
        "lw s4,  4 * 25(sp)\n"
        "lw s5,  4 * 26(sp)\n"
        "lw s6,  4 * 27(sp)\n"
        "lw s7,  4 * 28(sp)\n"
        "lw s8,  4 * 29(sp)\n"
        "lw s9,  4 * 30(sp)\n"
        "lw s10, 4 * 31(sp)\n"
        "lw s11, 4 * 32(sp)\n"
        "lw sp,  4 * 3(sp)\n"
        "mret\n"
    );
}

static void handle_timer_interrupt(void) {
    // reset interval(enable timer interrupt)
    do {
        *(uint32_t *)REG_MTIMECMP += INTERVAL;
    } while ((*(uint32_t *)REG_MTIME > *(uint32_t *)REG_MTIMECMP));

    // switch task
    task_switch();
}

// interrupt handler
void riscv32_handle_trap(trap_frame *f) {
    uint32_t mcause = READ_CSR(mcause);
    uint32_t mtval = READ_CSR(mtval);
    uint32_t mepc = READ_CSR(mepc);

    switch(mcause) {
        case TIMER_INTERRUPT:
            handle_timer_interrupt();
            break;
        
        default:
            PANIC(
                "unexpected trap mcause=%x, mtval=%x, mepc=%x\n", 
                mcause, 
                mtval, 
                mepc
            );
            break;
    }
}

void arch_set_trap_handlers(void) {
    WRITE_CSR(mtvec, (uint32_t) trap_entry);

    // set interval
    *(uint32_t *)REG_MTIMECMP = INTERVAL;

    // enable machine timer interrupt
    WRITE_CSR(mie, MIE_MTIE);

    // enale interrupt
    WRITE_CSR(mstatus, MSTATUS_MIE);

}