#include <kernel/runtime.h>
#include <arch/runtime.h>
#include <arch/cortex.h>
#include <cmrx/assert.h>

#ifdef __FPU_USED
void os_thread_initialize_arch(struct OS_thread_t * thread)
{
    // By default, thread is restored into
    // Thread mode, using PSP as a stack and
    // without FPU
    thread->arch.exc_return = EXC_RETURN_THREAD_PSP;
}

void os_save_fpu_context(struct OS_thread_t * thread)
{
    if (thread->arch.exc_return == EXC_RETURN_THREAD_PSP_FPU)
    {
        asm volatile(
            "VSTM R0!,{S16-S31}"
        );
    }
}

void os_load_fpu_context(struct OS_thread_t * thread)
{
    if (thread->arch.exc_return == EXC_RETURN_THREAD_PSP_FPU)
    {
        asm volatile(
            "VLDM R0,{S16-S31}"
        );
    }
}
#endif

/* FPU initialization routines for Cortex-M architecture */

void os_init_arch(void)
{
    // This function intentionally does nothing.
}

/* Configure the core for automatic floating-point state saving
 * and enable lazy FPU state saving to speed-up kernel execution.
 */
void os_init_core(unsigned core_id)
{
    (void) core_id;
    // Here we assume that FP coprocessor was enabled by the SystemInit from within HAL.
    FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk;
}
