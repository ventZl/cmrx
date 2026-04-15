/*
 * CMRX RISC-V machine timer ISR for Pico SDK (RP2350).
 *
 * Overrides the weak isr_riscv_machine_timer from crt0_riscv.S.
 * Saves a full ExceptionFrame so that a context switch during the
 * safe-point restores the new thread's complete register state.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmrx/arch/riscv/exception_frame.h>

__attribute__((naked, section(".time_critical.cmrx_timer_handler")))
void isr_riscv_machine_timer(void)
{
	SAVE_CONTEXT();
	asm volatile(
		"call cmrx_machine_timer_handler\n\t"
		"call os_riscv_context_switch_safe_point\n\t"
		::: "memory"
	);
	LOAD_CONTEXT();
	asm volatile("mret\n\t" ::: "memory");
}
