#include <ctest.h>

#include <cmrx/arch/riscv/hal.h>

CTEST(riscv_hal, csr_read_write_roundtrip)
{
    /* mstatus */
    cmrx_riscv_csr_write_mstatus(0x11223344u);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mstatus(), 0x11223344u);
    cmrx_riscv_csr_write_mstatus(0xa5a5a5a5u);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mstatus(), 0xa5a5a5a5u);

    /* mepc */
    cmrx_riscv_csr_write_mepc(0x01020304u);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mepc(), 0x01020304u);

    /* mcause */
    cmrx_riscv_csr_write_mcause(0x8000000bu);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mcause(), 0x8000000bu);

    /* mtvec */
    cmrx_riscv_csr_write_mtvec(0x00001000u);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mtvec(), 0x00001000u);

    /* mie */
    cmrx_riscv_csr_write_mie(0x00000088u);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mie(), 0x00000088u);

    /* mip */
    cmrx_riscv_csr_write_mip(0x00000008u);
    ASSERT_EQUAL(cmrx_riscv_csr_read_mip(), 0x00000008u);
}

CTEST(riscv_hal, irq_enable_disable)
{
    cmrx_riscv_irq_disable();
    ASSERT_EQUAL(cmrx_riscv_irq_is_enabled(), false);

    cmrx_riscv_irq_enable();
    ASSERT_EQUAL(cmrx_riscv_irq_is_enabled(), true);

    cmrx_riscv_irq_disable();
    ASSERT_EQUAL(cmrx_riscv_irq_is_enabled(), false);
}

