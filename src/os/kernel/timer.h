/** @defgroup os_timer Timers
 * @ingroup os
 * Kernel internals providing services of delayed and periodic execution.
 * @{
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "runtime.h"

/* List of various timer types supported by this framework.
 * For most part, the most important characteristic is if
 * the timer is periodic or not.
 *
 * This is decided by if the timer type is *above* or *below*
 * the TIMER_PERIODIC type.
 */
enum eSleepType {
    TIMER_SLEEP	= 0,
    TIMER_TIMEOUT,
    TIMER_PERIODIC,						// Marks the first periodic timer type
    TIMER_INTERVAL = TIMER_PERIODIC,
};

/** Description of one sleep request.
 * Contains details required to calculate when the next sleep interrupt shall happen
 * and to determine which request shall be the next.
 */
struct TimerEntry_t {
    uint32_t sleep_from;      ///< time at which sleep has been requested
    uint32_t interval;        ///< amount of time sleep shall take
    uint8_t thread_id;        ///< thread ID which requested the sleep
    uint8_t timer_type;       ///< type of sleep performed
};

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

/** Find a slot for timed event and store it.
 *
 * This is actual execution core for both \ref os_usleep and \ref os_setitimer
 * functions. It will find free slot or slot already occupied by calling thread
 * and will set / update timeout values.
 * @param microseconds time for which thread should sleep / wait for event
 * @param type type of timed event to set up
 * @returns error status. 0 means no error.
 */
int os_set_timed_event(unsigned microseconds, enum eSleepType type);

/** Cancels existing timed event.
 *
 * This function is only accessible for periodic timers externally. It
 * allows cancelling of interval timers set previously.
 * @param owner thread which shall own the interval timer
 * @param type type of the event to be cancelled
 * @return 0 if operation performed succesfully.
 */
int os_cancel_timed_event(Thread_t owner, enum eSleepType type);

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

/** Get current CPU frequency.
 *
 * Will return the current CPU frequency (if known and/or available).
 * @returns CPU frequency in Hz, 0 if value is not known.
 */
uint32_t os_cpu_freq_get(void);

/** @} */
