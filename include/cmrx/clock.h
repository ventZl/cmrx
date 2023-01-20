#pragma once

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

/** Perform tight busy-wait delay.
 * Kernel calls this function whenever tight delay is required. If delays are in orders
 * of microseconds, then it usually makes no sense to yield or try to do anything else
 * because the overhead of thread switching will consume all the time. Timing timing 
 * provider shall perform busy-wait delay, waiting for given amount of microseconds.
 * @param [in] delays_ns amount of microseconds to wait before resuming execution
 */
void timing_provider_delay(long delay_us);

/** The function timing provider has to call.
 */
long os_sched_timing_callback(long delay_us);

