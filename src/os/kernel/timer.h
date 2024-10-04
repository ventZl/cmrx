/** @defgroup os_timer Timers
 * @ingroup os
 * Kernel internals providing services of delayed and periodic execution.
 * @{
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Kernel implementation of usleep() syscall.
 * See \ref usleep for details on arguments.
 */
int os_usleep(unsigned microseconds);

/** Kernel implementation of setitimer() syscall.
 * See \ref setitimer for details on arguments.
 */
int os_setitimer(unsigned microseconds);

/** This routine initializes kernel scheduling subsystem.
 * It is necessary to call this routine before first call to
 * either timer syscalls, or \ref os_run_timer otherwise
 * things will go wrong.
 */
void os_timer_init();

/** Provide information on next scheduled event.
 *
 * This function informs caller about delay until next scheduled event.
 * Next scheduled event may be either wake-up of sleeped thread, or
 * interval timer.
 * @param [out] delay address of buffer, where delay to next scheduled event will be written
 * @returns true if there is any scheduled event known and at address pointed to by delay
 * value was written. Returns false if there is no known scheduled event. In such case content
 * of memory pointed to by delay is undefined.
 */
bool os_schedule_timer(unsigned * delay);

/** Fire scheduled event.
 *
 * Will find and run scheduled event.
 * @param microtime current processor time in microseconds
 */
void os_run_timer(uint32_t microtime);

/** @} */
