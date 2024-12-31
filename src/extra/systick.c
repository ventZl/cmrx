#include <extra/systick.h>
#include <RTE_Components.h>
#include <stdint.h>
#include <cmrx/clock.h>
#include CMSIS_device_header
#include <cmrx/sys/trace.h>

static uint32_t systick_us = 0;

static inline void SysTick_Enable()
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static inline void SysTick_Disable()
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void timing_provider_setup(int interval_ms)
{
    // We need the PendSV be of the same priority as SysTick.
    // Otherwise scheduling PendSV from SysTick will fire it 
    // immediately and that won't do any good to the state of
    // the program.
    NVIC_SetPriority(PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    SysTick_Config(SystemCoreClock / (interval_ms * 1000));
    // SysTick_Config will enable the systick automatically
    SysTick_Disable();
    systick_us = interval_ms * 1000;
}

__attribute__((interrupt)) void SysTick_Handler()
{
    trace_event(EVENT_KERNEL_TICK, systick_us);
    os_sched_timing_callback(systick_us);
}

void timing_provider_schedule(long delay_us)
{
    if (delay_us == 0)
    {
        SysTick_Disable();
    } else {
        SysTick_Enable();
    }

}

void timing_provider_delay(long delay_us)
{ 
    volatile uint32_t cycles_count = (SystemCoreClock / 1000000) * delay_us;

    // This usually takes 8 cycles to make one loop
    // The cycle count may change, so it should ideally be written in assembly.
    do {
    } while((cycles_count--) > 0);
    
    return;
}

