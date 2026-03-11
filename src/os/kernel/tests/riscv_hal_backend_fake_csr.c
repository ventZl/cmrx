#include <cmrx/arch/riscv/hal.h>

static uint32_t fake_mstatus;
static uint32_t fake_mepc;
static uint32_t fake_mcause;
static uint32_t fake_mtvec;
static uint32_t fake_mie;
static uint32_t fake_mip;

uint32_t cmrx_riscv_csr_read_mstatus(void) { return fake_mstatus; }
void cmrx_riscv_csr_write_mstatus(uint32_t value) { fake_mstatus = value; }

uint32_t cmrx_riscv_csr_read_mepc(void) { return fake_mepc; }
void cmrx_riscv_csr_write_mepc(uint32_t value) { fake_mepc = value; }

uint32_t cmrx_riscv_csr_read_mcause(void) { return fake_mcause; }
void cmrx_riscv_csr_write_mcause(uint32_t value) { fake_mcause = value; }

uint32_t cmrx_riscv_csr_read_mtvec(void) { return fake_mtvec; }
void cmrx_riscv_csr_write_mtvec(uint32_t value) { fake_mtvec = value; }

uint32_t cmrx_riscv_csr_read_mie(void) { return fake_mie; }
void cmrx_riscv_csr_write_mie(uint32_t value) { fake_mie = value; }

uint32_t cmrx_riscv_csr_read_mip(void) { return fake_mip; }
void cmrx_riscv_csr_write_mip(uint32_t value) { fake_mip = value; }
