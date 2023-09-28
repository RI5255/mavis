
#include "asm.h"
#include "trap.h"

#include <stdint.h>
#include <lib/common/print.h>
#include <kernel/task.h>

// interrupt entry
__attribute__((naked))
__attribute__((aligned(4)))
static void trap_entry(void) {
    __asm__ __volatile__(
        "csrw mscratch, a0\n"
        "mv a0, sp\n"
        "addi a0, a0, -8 * 33\n"
        "sw ra,  8 * 2(a0)\n"
        "sw sp,  8 * 3(a0)\n"
        "sw gp,  8 * 4(a0)\n"
        "sw tp,  8 * 5(a0)\n"
        "sw t0,  8 * 6(a0)\n"
        "sw t1,  8 * 7(a0)\n"
        "sw t2,  8 * 8(a0)\n"
        "sw t3,  8 * 9(a0)\n"
        "sw t4,  8 * 10(a0)\n"
        "sw t5,  8 * 11(a0)\n"
        "sw t6,  8 * 12(a0)\n"
        "sw a1,  8 * 14(a0)\n"
        "sw a2,  8 * 15(a0)\n"
        "sw a3,  8 * 16(a0)\n"
        "sw a4,  8 * 17(a0)\n"
        "sw a5,  8 * 18(a0)\n"
        "sw a6,  8 * 19(a0)\n"
        "sw a7,  8 * 20(a0)\n"
        "sw s0,  8 * 21(a0)\n"
        "sw s1,  8 * 22(a0)\n"
        "sw s2,  8 * 23(a0)\n"
        "sw s3,  8 * 24(a0)\n"
        "sw s4,  8 * 25(a0)\n"
        "sw s5,  8 * 26(a0)\n"
        "sw s6,  8 * 27(a0)\n"
        "sw s7,  8 * 28(a0)\n"
        "sw s8,  8 * 29(a0)\n"
        "sw s9,  8 * 30(a0)\n"
        "sw s10, 8 * 31(a0)\n"
        "sw s11, 8 * 32(a0)\n"

        "mv sp, a0\n"
        "csrr a0, mscratch\n"
        "sw a0,  8 * 13(sp)\n"
        
        "csrr  a0, mepc\n"
        "sw a0, 8 * 0(sp)\n"
        "csrr  a0, mstatus\n"
        "sw a0, 8 * 1(sp)\n"

        "mv a0, sp\n"
        "call riscv64_handle_trap\n"

        "lw a0, 8 * 0(sp)\n"
        "csrw mepc, a0\n"
        "lw a0, 8 * 1(sp)\n"
        "csrw mstatus, a0\n"

        "lw ra,  8 * 2(sp)\n"
        "lw gp,  8 * 4(sp)\n"
        "lw tp,  8 * 5(sp)\n"
        "lw t0,  8 * 6(sp)\n"
        "lw t1,  8 * 7(sp)\n"
        "lw t2,  8 * 8(sp)\n"
        "lw t3,  8 * 9(sp)\n"
        "lw t4,  8 * 10(sp)\n"
        "lw t5,  8 * 11(sp)\n"
        "lw t6,  8 * 12(sp)\n"
        "lw a0,  8 * 13(sp)\n"
        "lw a1,  8 * 14(sp)\n"
        "lw a2,  8 * 15(sp)\n"
        "lw a3,  8 * 16(sp)\n"
        "lw a4,  8 * 17(sp)\n"
        "lw a5,  8 * 18(sp)\n"
        "lw a6,  8 * 19(sp)\n"
        "lw a7,  8 * 20(sp)\n"
        "lw s0,  8 * 21(sp)\n"
        "lw s1,  8 * 22(sp)\n"
        "lw s2,  8 * 23(sp)\n"
        "lw s3,  8 * 24(sp)\n"
        "lw s4,  8 * 25(sp)\n"
        "lw s5,  8 * 26(sp)\n"
        "lw s6,  8 * 27(sp)\n"
        "lw s7,  8 * 28(sp)\n"
        "lw s8,  8 * 29(sp)\n"
        "lw s9,  8 * 30(sp)\n"
        "lw s10, 8 * 31(sp)\n"
        "lw s11, 8 * 32(sp)\n"
        "lw sp,  8 * 3(sp)\n"
        "mret\n"
    );
}

static void handle_timer_interrupt(void) {
    // reset interval(enable timer interrupt)
    do {
        *(uint64_t *)REG_MTIMECMP += INTERVAL;
    } while ((*(uint64_t *)REG_MTIME > *(uint64_t *)REG_MTIMECMP));

    // switch task
    task_switch();
}

// interrupt handler
void riscv64_handle_trap(struct trap_frame *f) {
    uint64_t mcause = READ_CSR(mcause);
    uint64_t mtval = READ_CSR(mtval);
    uint64_t mepc = READ_CSR(mepc);

    switch(mcause) {
        /*
        case TIMER_INTERRUPT:
            handle_timer_interrupt();
            break;
        */
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
    WRITE_CSR(mtvec, (uint64_t) trap_entry);
    /*
    // set interval
    *(uint64_t *)REG_MTIMECMP = INTERVAL;

    // enable machine timer interrupt
    WRITE_CSR(mie, MIE_MTIE);

    // enale interrupt
    WRITE_CSR(mstatus, MSTATUS_MIE);
    */
}