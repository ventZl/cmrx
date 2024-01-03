#pragma once
/** @defgroup api_timing Timing provider API 
 * @ingroup api 
 * API providing clock source to the kernel.
 * 
 * CMRX kernel needs source of clock to perform certain tasks, such as preemptive
 * multi-threading and reliable implementation of delays and timers. This resource
 * is not provided by the kernel itself. It is a task of the integrator to provide 
 * suitable implementation of calls which form the API of timing provider. This
 * gives the integrator a possibility to choose the source of clock which suits his
 * intended purposes the best.
 *
 * The design of timing provider API is done in a way that it is possible to implement
 * CPU core power management here as the call directly informs the integrator on how
 * long the kernel is not expecting any CPU activity. Integrator is free to perform
 * the delay in any way available on the target HW, including shutting down the CPU 
 * as long as the method implemented is able to resume CPU operation in requested time
 * or very close to it.
 *
 * Kernel source tree provides @ref aux_systick based on
 * ARM Cortex-M SysTick peripheral. This implementation lacks any form of power management
 * as the SysTick peripheral is only running if CPU clock is also running. It can be used
 * to bootstrap the kernel on the target HW or debug problems with more complex timing 
 * providers.
 *
 * @section Overview
 * 
 * The timing provider API allows kernel to perform two basic operations:
 *  * short term precise synchronous delays 
 *  * long term asynchronous delays involving power management
 *
 * The reason why kernel is using the timing provider API for both tasks is, that it doesn't
 * know the hardware it is running on. Implementation of clocks and power management differs
 * from chip to chip and it would be limiting to impose some way of clock control. Rather,
 * the kernel provides simple API the integrator can implement to best suite his purposes.
 * 
 * @section sync_delay Precise synchronous delays
 *
 * This kind of delays is used whenever kernel assumes that the delay timing is too short
 * to perform any other task. This typically happens with short - microsecond range - delays
 * in implementations of various protocols.
 *
 * The integrator has to provide an implementation of this call which performs as precise 
 * delay, as possible. This delay can be implemented as busy-wait and will be called in 
 * privileged kernel mode. Integrator should consider possible CPU power states (mainly
 * the current CPU frequency) to perform the delay correctly.
 *
 *
 * @section async_delay Asynchronous delays involving power management
 * 
 * This kind of delay is used when kernel needs to be notified that certain amount of time
 * has passed. This call is asynchronous in terms that the timing provider shall return 
 * immediately after it sets up timer to trigger after certain amount of time.
 *
 * Once the hardware timer triggers the delay, the timing provider shall call the 
 * @ref os_sched_timing_callback() of the kernel notifying it, that given amount of time 
 * has passed.
 * @{
 */

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

long os_sched_timing_callback(long delay_us);

/** @} */
