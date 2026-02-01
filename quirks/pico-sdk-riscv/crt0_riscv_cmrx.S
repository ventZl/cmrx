/*
 * CMRX RISC-V IRQ handler wrapper for Pico SDK (RP2350).
 *
 * Provides a replacement for isr_riscv_machine_external_irq that calls the
 * CMRX context switch safe-point at the IRQ exit boundary.
 *
 * This file is derived from pico-sdk crt0_riscv.S (BSD-3-Clause).
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico.h"
#include "hardware/regs/rvcsr.h"

.section .time_critical.cmrx_irq_handler, "ax"

/*
 * External IRQ handler with CMRX safe-point hook.
 *
 * This replaces the weak isr_riscv_machine_external_irq from crt0_riscv.S.
 * The structure mirrors the original but adds a call to the context switch
 * safe-point before returning from the IRQ.
 */
.global isr_riscv_machine_external_irq
isr_riscv_machine_external_irq:
    addi sp, sp, -80
    sw ra,  0(sp)
    sw t0,  4(sp)
    sw t1,  8(sp)
    sw t2, 12(sp)
    sw a0, 16(sp)
    sw a1, 20(sp)
    sw a2, 24(sp)
    sw a3, 28(sp)
    sw a4, 32(sp)
    sw a5, 36(sp)
    sw a6, 40(sp)
    sw a7, 44(sp)
    sw t3, 48(sp)
    sw t4, 52(sp)
    sw t5, 56(sp)
    sw t6, 60(sp)
    csrr a0, mepc
    csrr a1, mstatus
    sw a0, 64(sp)
    sw a1, 68(sp)
cmrx_save_meicontext:
    csrrsi a2, RVCSR_MEICONTEXT_OFFSET, RVCSR_MEICONTEXT_CLEARTS_BITS
    sw a2, 72(sp)

cmrx_get_first_irq:
    csrrsi a0, RVCSR_MEINEXT_OFFSET, RVCSR_MEINEXT_UPDATE_BITS
    bltz a0, cmrx_no_more_irqs
cmrx_dispatch_irq:
    csrsi mstatus, 0x8
    lui a1, %hi(__soft_vector_table)
    add a1, a1, a0
    lw a1, %lo(__soft_vector_table)(a1)
    jalr ra, a1
    csrci mstatus, 0x8
cmrx_get_next_irq:
    csrrsi a0, RVCSR_MEINEXT_OFFSET, RVCSR_MEINEXT_UPDATE_BITS
    bgez a0, cmrx_dispatch_irq

cmrx_no_more_irqs:
    /*
     * All IRQs dispatched. Before restoring context and returning,
     * call the CMRX context switch safe-point. This allows a pending
     * context switch (requested by the scheduler during IRQ handling)
     * to be performed now, at a safe boundary.
     *
     * The safe-point function preserves s0-s11 per RISC-V calling convention.
     * Caller-saved registers (a0, t0, etc.) may be clobbered, but we restore
     * them from the stack frame below, so this is safe.
     */
    call os_riscv_context_switch_safe_point

    /* Restore saved context and return from IRQ */
    lw a0, 64(sp)
    lw a1, 68(sp)
    lw a2, 72(sp)
    csrw mepc, a0
    csrw mstatus, a1
    csrw RVCSR_MEICONTEXT_OFFSET, a2
    lw ra,  0(sp)
    lw t0,  4(sp)
    lw t1,  8(sp)
    lw t2, 12(sp)
    /* skip a0 for now */
    lw a1, 20(sp)
    lw a2, 24(sp)
    lw a3, 28(sp)
    lw a4, 32(sp)
    lw a5, 36(sp)
    lw a6, 40(sp)
    lw a7, 44(sp)
    lw t3, 48(sp)
    lw t4, 52(sp)
    lw t5, 56(sp)
    lw t6, 60(sp)

cmrx_check_irq_before_exit:
    /*
     * Optimization: check for new IRQ before completing the mret sequence.
     * If an IRQ arrived while we were in the safe-point, handle it now
     * rather than taking the mret->enter->save latency.
     */
    csrr a0, RVCSR_MEINEXT_OFFSET
    bgez a0, cmrx_save_meicontext
    lw a0, 16(sp)
    addi sp, sp, 80
    mret
