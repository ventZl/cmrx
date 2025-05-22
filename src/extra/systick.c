#include <extra/systick.h>
#include <RTE_Components.h>
#include <stdint.h>
#include <cmrx/clock.h>
#include CMSIS_device_header

static uint32_t systick_us = 0;

static inline void SysTick_Enable(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static inline void SysTick_Disable(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

static inline uint32_t SysTick_SetTimeout(uint32_t ticks)
{
    if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
    {
        return (1UL);                                                   /* Reload value impossible */
    }

    SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         /* set reload register */
    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
    SysTick_CTRL_TICKINT_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
    return (0UL);
}

void timing_provider_setup(int interval_ms)
{
    // We need the PendSV be of the same priority as SysTick.
    // Otherwise scheduling PendSV from SysTick will fire it 
    // immediately and that won't do any good to the state of
    // the program.
    NVIC_SetPriority(PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    SysTick_SetTimeout(SystemCoreClock / (interval_ms * 1000));
    // SysTick_Config will enable the systick automatically
    SysTick_Disable();
    systick_us = interval_ms * 1000;
}

__attribute__((interrupt)) void SysTick_Handler()
{
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

