/** @addtogroup os_timer 
 * @{ 
 */
#include <cmrx/os/timer.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/signal.h>
#include <conf/kernel.h>

#include <stdint.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>

#include <stdbool.h>
#include <cmrx/clock.h>

/** Description of one sleep request.
 * Contains details required to calculate when the next sleep interrupt shall happen
 * and to determine which request shall be the next.
 */
struct TimerEntry_t {
	uint32_t sleep_from;      ///< time at which sleep has been requested
	uint32_t interval;        ///< amount of time sleep shall take
	uint8_t thread_id;        ///< thread ID which requested the sleep
};

/** List of all delays requested from kernel.
 * This structure contains all scheduled sleeps requested by all threads.
 * Every thread can technically request one periodic timer and one one-shot
 * delay.
 */
struct TimerEntry_t sleepers[SLEEPERS_MAX];

/** Determine if interval in sleep entry is periodic or not.
 * @returns 1 if interval is periodic, 0 if interval is one-shot.
 */
#define IS_PERIODIC(interval) (((interval) >> 31) == 1)

/** Get how long the sleep should take.
 * Will clean the periodicity flag from interval.
 * @returns sleep time in microseconds.
 */
#define GET_SLEEPTIME(interval) ((interval) & (~(1 << 31)))

/** Perform heavy lifting of setting timers
 * This routine will store timed event into list of timed events. If 
 * event is not periodic, it will also stop thread (\ref os_usleep semantics).
 *
 * @param slot number of slot in \ref sleepers list
 * @param interval amount of us for which thread should sleep
 * @param _periodic true if timer should be periodic, false otherwise
 * @return 0 if timer was set up properly
 */
static int do_set_timed_event(unsigned slot, unsigned interval, bool _periodic)
{
	uint32_t periodic = (_periodic ? 1 : 0);
	Thread_t thread_id = os_get_current_thread();
	uint32_t microtime = os_get_micro_time();
	sleepers[slot].thread_id = thread_id;
	ASSERT(sleepers[slot].thread_id < OS_THREADS);
	sleepers[slot].sleep_from = microtime;
	sleepers[slot].interval = interval | (periodic << 31);
	if (!_periodic)
	{
		os_thread_stop(thread_id);
	}
	return 0;
}

/** Find a slot for timed event and store it.
 *
 * This is actual execution core for both \ref os_usleep and \ref os_setitimer 
 * functions. It will find free slot or slot already occupied by calling thread
 * and will set / update timeout values.
 * @param microseconds time for which thread should sleep / wait for event
 * @param periodic true if this event is periodic
 * @returns error status. 0 means no error.
 */
static int set_timed_event(unsigned microseconds, bool periodic)
{
	Thread_t thread_id = os_get_current_thread();

	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id == 0xFF)
		{
			return do_set_timed_event(q, microseconds, periodic);
		}
		else
		{
			if (sleepers[q].thread_id == thread_id)
			{
				/* We are searching for interval timer / delay for this thread
				 * we found some entry belonging to this thread, but it is of
				 * different kind than we are searching for. It is delay and
				 * we are searching for interval timer or vice versa.
				 */
				if (
						(periodic  && IS_PERIODIC(sleepers[q].interval))
						|| (!periodic && !(IS_PERIODIC(sleepers[q].interval)))
				   )
				{
					return do_set_timed_event(q, microseconds, periodic);
				}
			}
		}
	}
	return E_NOTAVAIL;
}

/** Cancels existing timed event.
 *
 * This function is only accessible for periodic timers externally. It
 * allows cancelling of interval timers set previously.
 * @param owner thread which shall own the interval timer
 * @param periodic true if periodic timer of given thread should be cancelled
 * @return 0 if operation performed succesfully.
 */
static int cancel_timed_event(Thread_t owner, bool periodic)
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id == owner)
		{
			if (periodic == IS_PERIODIC(sleepers[q].interval))
			{
				sleepers[q].thread_id = 0xFF;
				return 0;
			}
		}
	}

	return E_NOTAVAIL;
}

/** Perform short busy wait.
 * This will perform busywait in the context of current thread.
 * Useful to do short waits for I/O.
 * @param period_us how long the wait should take
 */
static void delay_us(uint32_t period_us)
{
    timing_provider_delay(period_us);
}

int os_usleep(unsigned microseconds)
{
	if (microseconds < 1000)
	{
		delay_us(microseconds);
		return 0;
	}
	return set_timed_event(microseconds, false);
}

int os_setitimer(unsigned microseconds)
{
	if (microseconds > 0)
	{
		return set_timed_event(microseconds, true);
	}
	else
	{
		return cancel_timed_event(os_get_current_thread(), true);
	}
}

void os_timer_init()
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		sleepers[q].thread_id = 0xFF;
	}
}

/** Helper function to calculate time considering type wraparound.
 * This function will calculate the final time considering timer wraparound.
 * @param sleep_from time at which the sleep starts (microseconds)
 * @param microtime duration of sleep (microseconds)
 * @returns new value of kernel timer at the end of sleep
 */
static uint32_t get_sleep_time(uint32_t sleep_from, uint32_t microtime)
{
	if (sleep_from < microtime)
	{
		return microtime - sleep_from;
	}
	else
	{
		return ~0 - (sleep_from - microtime);
	}
}

bool os_schedule_timer(unsigned * delay)
{
	uint32_t microtime = os_get_micro_time();
	uint32_t min_delay = ~0;
	bool rv = false;

	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id != 0xFF)
		{
			/* Figure how long this particular sleeper is already sleeing */
			uint32_t sleeping = get_sleep_time(sleepers[q].sleep_from, microtime);
			/* Figure out how long should this particular sleeper continue
			 * to sleep
			 */
			uint32_t tosleep = GET_SLEEPTIME(sleepers[q].interval);

			uint32_t delay;
			if (sleeping < tosleep)
			{
				delay = tosleep - sleeping;
			}
			else
			{
				/* This timer is overdue, fire immediately */
				delay = 0;
			}

			if (delay < min_delay)
			{
				min_delay = delay;
				rv = true;
			}
		}
	}
	if (rv)
	{
		*delay = min_delay;
	}
	
	return rv;
}

void os_run_timer(uint32_t microtime)
{

	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id != 0xFF)
		{

			if (get_sleep_time(sleepers[q].sleep_from, microtime) >= GET_SLEEPTIME(sleepers[q].interval))
			{
				// restart usleep-ed thread
				os_thread_continue(sleepers[q].thread_id);
				if (IS_PERIODIC(sleepers[q].interval))
				{
					sleepers[q].sleep_from = sleepers[q].sleep_from + GET_SLEEPTIME(sleepers[q].interval);
				}
				else
				{
					sleepers[q].thread_id = 0xFF;
				}
			}
		}
	}
}

/** @} */
