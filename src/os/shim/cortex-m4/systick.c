#include <cmrx/shim/systick.h>
#include <cmrx/shim/clksource.h>
#include <stdint.h>
#include <libopencm3/cm3/systick.h>

/** Amount of real time advance per one scheduler tick. */
uint32_t sched_tick_increment = 0;

/*
 * Set up timer to fire every x milliseconds
 * This is a unusual usage of systick, be very careful with the 24bit range
 * of the systick counter!  You can range from 1 to 2796ms with this.
 */
void systick_setup(int xms)
{
	uint32_t core_freq;

	core_freq = sysclk_freq();
	sched_tick_increment = xms * 1000;

	/* div8 per ST, stays compatible with M3/M4 parts, well done ST */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	/* clear counter so it starts right away */

	systick_set_reload(core_freq / 8 / 1000 * xms);
	STK_CVR = 0;
	systick_interrupt_enable();
}

void systick_enable()
{
	systick_counter_enable();
}

  
