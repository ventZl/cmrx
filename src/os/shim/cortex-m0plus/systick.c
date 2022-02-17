#include <cmrx/shim/systick.h>
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
	uint32_t ahb_freq;


#warning "There is frequency value hardcoded here!"
	ahb_freq = 4000000; //rcc_ahb_frequency;
	sched_tick_increment = xms * 1000;

	systick_set_reload(ahb_freq / 8 / 1000 * xms);
	STK_CVR = 0;
	systick_interrupt_enable();
}

void systick_enable()
{
	systick_counter_enable();
}

void systick_disable()
{
	systick_counter_disable();
}
