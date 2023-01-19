#pragma once

/** This is auxiliary implementation of timing provider using ARM SysTick.
 * This file can serve both as an example of how to implement timing provider 
 * and also serve as quick bootstrap timing provider for your project. You'll 
 * have to replace it if you want to either go tickless or implement power
 * management.
 */

/** Setup the timing provider.
 * This function shall be called by the main() function during hardware bringup.
 * Its purpose is to configure whatever peripheral or combination of peripherals 
 * that provides kernel timing services. These peripherals shall be configured
 * and prepared, but not launched yet.
 *
 * Kernel never calls this function on its own. The prototype of this function
 * can be modified by the integrator to suit all the needs.
 *
 * This function shall be called before the @ref os_start() function is called
 * and can therefore be called in any suitable context.
 *
 * @param [in] interval_ms how often the systick ISR shall be called.
 */
void timing_provider_setup(int interval_ms);

/** SysTick interrupt handler.
 * This ISR serves SysTick interrupt. It contains a counter, how many more 
 * interrupts have to happen before kernel gets called. If counter reaches
 * zero, then kernel is called.
 * Return value from kernel denotes next delay before kernel shall be called
 * again. If that value is zero, then systick timer is disabled. Otherwise
 * the counter is restored with new value and cycle repeats again.
 * 
 * Different timing provider may choose to use different clock provider, or 
 * even providers and decide, that it will put CPU into sleep mode, if 
 * delay is too long, or there is no expected timed wake-up. If timing 
 * provider behaves as this, it is integrator's responsibility to ensure,
 * that external interrupts, or whatever other sources of events are enabled
 * in sleep state, which the timing provider will enter.
 *
 * Kernel never calls this function on its own. Implementor may use whatever
 * means of triggering the timer appropriate for given hardware. Whatever 
 * implementation provided must call kernel callback @ref 
 * os_sched_timing_callback from kernel-equivalent context.
 */
void sys_tick_handler();

/** Schedule next timer callback to kernel.
 * Re-configure timing provider so that next timed kernel call will happen after
 * given delay. This is the only function, whose implementation is mandatory and
 * whose prototype nor behavior cannot be changed.
 * If kernel calls this function, then timing provider must reconfigure itself
 * so that it will be able to call the kernel as close as possible to time 
 * @ref delay_us microseconds in the future. This must happen even if kernel
 * previously signalized that there is no expected timed event and timers were 
 * turned off entirely.
 * @param [in] delay_us delay time, in microseconds after which kernel shall be 
 * called by the timing provider.
 * @note This function is guarranteed to be called from kernel context.
 */
void timing_provider_schedule(long delay_us);
