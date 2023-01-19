#include <aux/systick.h>
#include <RTE_Components.h>
#include <stdint.h>
#include <cmrx/os/sched.h>
#include CMSIS_device_header

static uint32_t systick_us = 0;

void timing_provider_setup(int interval_ms)
{
    SysTick_Config(SystemCoreClock / (interval_ms * 1000));
    systick_us = interval_ms * 1000;
}

void SysTick_Handler()
{
    os_sched_timing_callback(systick_us);
}

void timing_provider_schedule(long delay_us)
{
    /* Do nothing */ 

}

