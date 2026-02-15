#include <ctest.h>

#include <cmrx/arch/riscv/hal.h>

/* MIE is bit 3 in mstatus (RISC-V Privileged Specification, Section 3.1.6.1). */
#define CMRX_RISCV_MSTATUS_MIE_MASK (1u << 3u)

CTEST(riscv_hal_backend, irq_enable_sets_mie_only)
{
	const uint32_t base = 0x5a5a5a5au & ~CMRX_RISCV_MSTATUS_MIE_MASK;

	cmrx_riscv_csr_write_mstatus(base);
	cmrx_riscv_irq_enable();

	const uint32_t after = cmrx_riscv_csr_read_mstatus();
	ASSERT_EQUAL(after & CMRX_RISCV_MSTATUS_MIE_MASK, CMRX_RISCV_MSTATUS_MIE_MASK);
	ASSERT_EQUAL(after & ~CMRX_RISCV_MSTATUS_MIE_MASK, base & ~CMRX_RISCV_MSTATUS_MIE_MASK);
	ASSERT_EQUAL(cmrx_riscv_irq_is_enabled(), true);
}

CTEST(riscv_hal_backend, irq_disable_clears_mie_only)
{
	const uint32_t base = 0xa5a5a5a5u | CMRX_RISCV_MSTATUS_MIE_MASK;

	cmrx_riscv_csr_write_mstatus(base);
	cmrx_riscv_irq_disable();

	const uint32_t after = cmrx_riscv_csr_read_mstatus();
	ASSERT_EQUAL(after & CMRX_RISCV_MSTATUS_MIE_MASK, 0u);
	ASSERT_EQUAL(after & ~CMRX_RISCV_MSTATUS_MIE_MASK, base & ~CMRX_RISCV_MSTATUS_MIE_MASK);
	ASSERT_EQUAL(cmrx_riscv_irq_is_enabled(), false);
}
