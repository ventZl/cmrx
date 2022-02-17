#include <cmrx/shim/systick.h>
#include <stdint.h>
#include <libopencm3/cm3/systick.h>
#warning "This stuff is hardcoded to use STM32 RCC"
#include <libopencm3/stm32/rcc.h>

/*
 * Set up timer to fire every x milliseconds
 * This is a unusual usage of systick, be very careful with the 24bit range
 * of the systick counter!  You can range from 1 to 2796ms with this.
 */
void systick_setup(int xms)
{
	uint32_t ahb_freq;

	ahb_freq = rcc_ahb_frequency;
	sched_tick_increment = xms * 1000;

	/* div8 per ST, stays compatible with M3/M4 parts, well done ST */
/*	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);*/
	/* clear counter so it starts right away */

	systick_set_reload(ahb_freq / 8 / 1000 * xms);
	STK_CVR = 0;
	systick_interrupt_enable();
}


