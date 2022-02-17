#include <cmrx/shim/systick.h>
#include <stdint.h>

/** Amount of real time advance per one scheduler tick. */
uint32_t sched_tick_increment = 0;

/*
 * Set up timer to fire every x milliseconds
 * This is a unusual usage of systick, be very careful with the 24bit range
 * of the systick counter!  You can range from 1 to 2796ms with this.
 */
void systick_setup(int xms)
{
	uint32_t ahb_freq;

#warning "This is hardcoded to use RCC value"

	ahb_freq = rcc_get_bus_clk_freq(RCC_AHBCLK);

	systick_set_frequency(1000/xms, ahb_freq);
	sched_tick_increment = xms * 1000;

	STK_CVR = 0;
	systick_interrupt_enable();
}


