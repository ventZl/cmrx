#include <cmrx/arch/riscv/hal.h>

uint32_t cmrx_riscv_csr_read_mstatus(void)
{
	uint32_t value = 0u;
	__asm__ volatile("csrr %0, mstatus" : "=r"(value));
	return value;
}

void cmrx_riscv_csr_write_mstatus(uint32_t value)
{
	__asm__ volatile("csrw mstatus, %0" : : "r"(value));
}

uint32_t cmrx_riscv_csr_read_mepc(void)
{
	uint32_t value = 0u;
	__asm__ volatile("csrr %0, mepc" : "=r"(value));
	return value;
}

void cmrx_riscv_csr_write_mepc(uint32_t value)
{
	__asm__ volatile("csrw mepc, %0" : : "r"(value));
}

uint32_t cmrx_riscv_csr_read_mcause(void)
{
	uint32_t value = 0u;
	__asm__ volatile("csrr %0, mcause" : "=r"(value));
	return value;
}

void cmrx_riscv_csr_write_mcause(uint32_t value)
{
	__asm__ volatile("csrw mcause, %0" : : "r"(value));
}

uint32_t cmrx_riscv_csr_read_mtvec(void)
{
	uint32_t value = 0u;
	__asm__ volatile("csrr %0, mtvec" : "=r"(value));
	return value;
}

void cmrx_riscv_csr_write_mtvec(uint32_t value)
{
	__asm__ volatile("csrw mtvec, %0" : : "r"(value));
}

uint32_t cmrx_riscv_csr_read_mie(void)
{
	uint32_t value = 0u;
	__asm__ volatile("csrr %0, mie" : "=r"(value));
	return value;
}

void cmrx_riscv_csr_write_mie(uint32_t value)
{
	__asm__ volatile("csrw mie, %0" : : "r"(value));
}

uint32_t cmrx_riscv_csr_read_mip(void)
{
	uint32_t value = 0u;
	__asm__ volatile("csrr %0, mip" : "=r"(value));
	return value;
}

void cmrx_riscv_csr_write_mip(uint32_t value)
{
	__asm__ volatile("csrw mip, %0" : : "r"(value));
}
