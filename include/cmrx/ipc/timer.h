#pragma once

#include <cmrx/os/sysenter.h>

/** Suspend thread execution for short time.
 *
 * Suspends thread execution for given amount of time. After this amount of time
 * elapses, thread execution is resumed. Suspend is one-time and usleep() has to be
 * called again in order to sleep again.
 * @param microseconds amount of microseconds (1/100000 s) to sleep
 * @note Actual amount of time which thread will be suspended depends on system tick
 * granularity and status of scheduler at the time of wake-up. If there is another
 * thread running/ready with equal/higher priority, then actual wake-up may be delayed
 * by the time consumed by this high-priority thread.
 */
__SYSCALL int usleep(unsigned microseconds);

/** Set interval timer to be called periodically.
 *
 * Configures interval timer, which will periodically be called, until cancelled.
 * Timer is called by delivering the SIGTIMER signal to the thread. If thread is stopped,
 * then SIGTIMER will resume it and call signal handler. If there is no signal handler
 * configured, then thread will just be resumed.
 * @param microseconds period of timer signals received. If set to 0, then interval timer
 * is cancelled.
 */
__SYSCALL int setitimer(unsigned microseconds);
