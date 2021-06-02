#include <cmrx/os/timer.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/signal.h>
#include <conf/kernel.h>

#include <stdint.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>

#include <stdbool.h>

struct TimerEntry_t {
	uint32_t sleep_from;
	uint32_t interval;
	uint8_t thread_id;
};

struct TimerEntry_t sleepers[SLEEPERS_MAX];

#define IS_PERIODIC(interval) (((interval) >> 31) == 1)
#define GET_SLEEPTIME(interval) ((interval) & (~(1 << 31)))

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

static int cancel_timed_event(Thread_t owner, bool periodic)
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id == owner)
		{
			if (
					(periodic && !IS_PERIODIC(sleepers[q].interval))
					|| (!periodic && IS_PERIODIC(sleepers[q].interval))
			   )
			{
				sleepers[q].thread_id = 0xFF;
				return 0;
			}
		}
	}

	return E_NOTAVAIL;
}

/** Kernel implementation of usleep() syscall */
int os_usleep(unsigned microseconds)
{
	return set_timed_event(microseconds, false);
}

/** Kernel implementation of setitimer() syscall */
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

/** Fire scheduled event.
 *
 * Will find and run scheduled event.
 * @param microtime current processor time in microseconds
 */
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
