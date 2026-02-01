#include <cmrx/arch/riscv/hal.h>

/* MIE is bit 3 in mstatus (riscv-privileged architecture document, Figure 3.6, Section 3.1.6.1, p.21). */
#define CMRX_RISCV_MSTATUS_MIE_MASK (1u << 3u)

void cmrx_riscv_irq_disable(void)
{
	const uint32_t mstatus = cmrx_riscv_csr_read_mstatus();
	cmrx_riscv_csr_write_mstatus(mstatus & ~CMRX_RISCV_MSTATUS_MIE_MASK);
}

void cmrx_riscv_irq_enable(void)
{
	const uint32_t mstatus = cmrx_riscv_csr_read_mstatus();
	cmrx_riscv_csr_write_mstatus(mstatus | CMRX_RISCV_MSTATUS_MIE_MASK);
}

bool cmrx_riscv_irq_is_enabled(void)
{
	return (cmrx_riscv_csr_read_mstatus() & CMRX_RISCV_MSTATUS_MIE_MASK) != 0u;
}
