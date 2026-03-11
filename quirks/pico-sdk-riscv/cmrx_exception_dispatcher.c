/*
 * CMRX RISC-V exception dispatcher for Pico SDK (RP2350).
 *
 * Overrides the weak isr_riscv_machine_exception from
 * exception_table_riscv.S.  Uses the arch-level SAVE_CONTEXT /
 * LOAD_CONTEXT macros for the full ExceptionFrame, adding only the
 * SDK-specific mscratch protocol and exception table dispatch.
 *
 * Individual exception handlers receive the ExceptionFrame pointer in a0.
 *
 * This file is derived from pico-sdk exception_table_riscv.S.
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmrx/arch/riscv/exception_frame.h>

__attribute__((naked, section(".time_critical.cmrx_exception_handler")))
void isr_riscv_machine_exception(void)
{
	/* Pico SDK mscratch protocol: swap ra with mscratch for nested detection */
	asm volatile(
		"csrrw ra, mscratch, ra\n\t"
		"bnez ra, __halt_on_unhandled_exception\n\t"
		::: "memory"
	);

	SAVE_CONTEXT();

	/* Fix ra: SAVE_CONTEXT stored ra=0; overwrite with actual ra from mscratch */
	asm volatile(
		"csrr t0, mscratch\n\t"
		"sw t0, " RISCV_EFS(EF_RA) "(sp)\n\t"

		"csrr t0, mcause\n\t"
		"li t1, 11\n\t"
		"bgtu t0, t1, 1f\n\t"

		"la t1, __riscv_exception_table\n\t"
		"slli t0, t0, 2\n\t"
		"add t0, t0, t1\n\t"
		"lw t0, (t0)\n\t"
		"mv a0, sp\n\t"
		"jalr ra, t0\n\t"

		"call os_riscv_context_switch_safe_point\n\t"
		"j 2f\n\t"

		"1:\n\t"
		"csrr ra, mscratch\n\t"
		"j __halt_on_unhandled_exception\n\t"

		"2:\n\t"
		::: "memory"
	);

	LOAD_CONTEXT();
	asm volatile(
		"csrw mscratch, zero\n\t"
		"mret\n\t"
		::: "memory"
	);
}
