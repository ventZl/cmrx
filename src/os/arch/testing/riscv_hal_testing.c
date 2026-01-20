/*
 * RISC-V HAL backend used by unit tests.
 *
 * This backend provides in-memory fake CSR values and interrupt state so the HAL
 * API can be tested on the host without any vendor headers or hardware.
 */

/*
 * NOTE: Some unit-test build configurations may redefine `static` for testability.
 * This backend must keep its fakes translation-unit local, so we explicitly undo
 * that here if needed.
 */
#ifdef static
#undef static
#endif

#include <cmrx/arch/riscv/hal.h>

static uint32_t fake_mstatus;
static uint32_t fake_mepc;
static uint32_t fake_mcause;
static uint32_t fake_mtvec;
static uint32_t fake_mie;
static uint32_t fake_mip;

/* IRQ enable state is represented as a uint32_t fake (0/1) per requirements. */
static uint32_t fake_irq_enabled;

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

void cmrx_riscv_irq_disable(void) { fake_irq_enabled = 0u; }
void cmrx_riscv_irq_enable(void) { fake_irq_enabled = 1u; }
bool cmrx_riscv_irq_is_enabled(void) { return fake_irq_enabled != 0u; }

