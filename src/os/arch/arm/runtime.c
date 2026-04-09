#include <kernel/runtime.h>
#include <arch/runtime.h>
#include <arch/cortex.h>
#include <cmrx/assert.h>
#include <kernel/sched.h>
#include <arch/corelocal.h>

#if __FPU_USED
void os_save_fpu_context(struct OS_thread_t * thread)
{
    ASSERT(cortex_is_thread_psp_used(thread->arch.exc_return));
    if (cortex_is_fpu_used(thread->arch.exc_return))
    {
        uint32_t * sp = thread->sp;
        asm volatile(
            "VSTMDB %0!,{S16-S31}"
            : "+r" (sp)
        );
    }
    // Here the lazy saving of FPU is active
    // This should not happen as we either triggered the save by above if block
    // or it shouldn't be active at all
    ASSERT((FPU->FPCCR & FPU_FPCCR_LSPACT_Msk) == 0);
}

void os_load_fpu_context(struct OS_thread_t * thread)
{
    // Here the lazy saving of FPU is still active
    ASSERT((FPU->FPCCR & FPU_FPCCR_LSPACT_Msk) == 0);
    if (cortex_is_fpu_used(thread->arch.exc_return))
    {
        uint32_t * sp = thread->sp;
        sp -= 15;
        asm volatile(
            "VLDMIA %0!,{S16-S31}"
            : "+r" (sp)
        );
    }
}

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
#if __FPU_USED
    // Here we assume that FP coprocessor was enabled by the SystemInit from within HAL.
    FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk;
#endif
}

bool os_is_thread_using_fpu(Thread_t thread_id)
{
    return cortex_is_fpu_used(os_threads[thread_id].arch.exc_return);
}

bool os_fpu_exception_frame(void)
{
    return os_is_thread_using_fpu(os_get_current_thread());
}

#endif

