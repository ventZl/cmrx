#pragma once

#include <conf/kernel.h>
#include <cmrx/arch/riscv/hal.h>

/*
 * RISC-V corelocal portability layer.
 *
 * For single-core builds, these are simple macros.
 * For SMP builds, actual implementations would be needed.
 */

typedef void (*callback_t)();

extern void os_core_sleep(void);

#ifndef CMRX_ARCH_SMP_SUPPORTED

#   define coreid()          0
#   define OS_NUM_CORES      1
#   define os_smp_lock()
#   define os_smp_unlock()

/*
 * Core lock/unlock via interrupt disable/enable.
 * Uses RISC-V HAL primitives.
 */
static inline void os_core_lock(void)
{
    cmrx_riscv_irq_disable();
}

static inline void os_core_unlock(void)
{
    cmrx_riscv_irq_enable();
}

#else

/* SMP mode - not implemented yet */
extern unsigned coreid(void);
extern void os_core_lock(void);
extern void os_core_unlock(void);
extern void os_smp_lock(void);
extern void os_smp_unlock(void);

#ifndef OS_NUM_CORES
#error "Macro OS_NUM_CORES is not defined. Use -DOS_NUM_CORES=x to tell the CMRX kernel how many cores it manages!"
#endif

#endif /* CMRX_ARCH_SMP_SUPPORTED */

