/*
 * Thin internal RISC-V HAL (CMSIS-like) for CMRX.
 *
 * Purpose:
 * - Provide a minimal, swappable API for CSR and interrupt primitives needed by
 *   the RISC-V thread switcher and its safe-point logic.
 *
 * Notes:
 * - This header intentionally does not define CSR bit layouts or trap semantics.
 * - Unit-test builds use a testing backend implemented with static fakes.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* ---- Minimal CSR accessors (subset justified by switcher needs) ---- */

uint32_t cmrx_riscv_csr_read_mstatus(void);
void cmrx_riscv_csr_write_mstatus(uint32_t value);

uint32_t cmrx_riscv_csr_read_mepc(void);
void cmrx_riscv_csr_write_mepc(uint32_t value);

uint32_t cmrx_riscv_csr_read_mcause(void);
void cmrx_riscv_csr_write_mcause(uint32_t value);

uint32_t cmrx_riscv_csr_read_mtvec(void);
void cmrx_riscv_csr_write_mtvec(uint32_t value);

uint32_t cmrx_riscv_csr_read_mie(void);
void cmrx_riscv_csr_write_mie(uint32_t value);

uint32_t cmrx_riscv_csr_read_mip(void);
void cmrx_riscv_csr_write_mip(uint32_t value);

/* ---- Interrupt primitives ---- */

void cmrx_riscv_irq_disable(void);
void cmrx_riscv_irq_enable(void);
bool cmrx_riscv_irq_is_enabled(void);

/* ---- Minimal barriers ---- */

static inline void cmrx_riscv_compiler_barrier(void)
{
    __asm__ volatile("" ::: "memory");
}

