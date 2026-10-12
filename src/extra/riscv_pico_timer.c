#include <extra/riscv_pico_timer.h>
#include <cmrx/clock.h>
#include <cmrx/arch/riscv/hal.h>
#include <hardware/riscv_platform_timer.h>
#include <stdint.h>
#include <stdbool.h>

/* MTIE is bit 7 in mie (riscv-privileged architecture document, Figure 3.15, Section 3.1.9, p.32). */
#define CMRX_RISCV_MIE_MTIE_MASK (1u << 7u)

static uint32_t timing_interval_us;

static void riscv_platform_mtie_enable(void)
{
    const uint32_t mie = cmrx_riscv_csr_read_mie();
    cmrx_riscv_csr_write_mie(mie | CMRX_RISCV_MIE_MTIE_MASK);
}

static void riscv_platform_mtie_disable(void)
{
    const uint32_t mie = cmrx_riscv_csr_read_mie();
    cmrx_riscv_csr_write_mie(mie & ~CMRX_RISCV_MIE_MTIE_MASK);
}

void cmrx_machine_timer_handler(void)
{
    /* re-arm mtimecmp for the next tick */
    uint64_t now = riscv_timer_get_mtime();
    riscv_timer_set_mtimecmp(now + timing_interval_us);

    /* hand control to the scheduler */
    os_sched_timing_callback(timing_interval_us);
}

void timing_provider_setup(int interval_ms)
{
    timing_interval_us = interval_ms * 1000UL;
}

void timing_provider_schedule(long delay_us)
{
    /* Only delay_us == 0 (stop) versus non-zero (run) is honored. The tick
     * period is fixed by timing_provider_setup(), so the requested duration
     * is not used for power management.
     */
    if (delay_us == 0) {
        riscv_platform_mtie_disable();
    } else {
        uint64_t now = riscv_timer_get_mtime();
        riscv_timer_set_mtimecmp(now + timing_interval_us);
        riscv_platform_mtie_enable();
    }
}

void timing_provider_delay(long delay_us)
{
    const uint64_t start = riscv_timer_get_mtime();
    uint64_t now = start;

    while ((int64_t)(now - start) < (int64_t)delay_us) {
        now = riscv_timer_get_mtime();
    }
}
