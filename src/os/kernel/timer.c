#include <cmrx/os/timer.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/signal.h>
#include <conf/kernel.h>

#include <stdint.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>

#include <stdbool.h>

struct SleepEntry_t {
	uint32_t sleep_to;
	uint8_t thread_id;
};

struct TimerEntry_t {
	uint32_t interval;
	uint32_t offset;
	uint8_t thread_id;
};

struct SleepEntry_t sleepers[SLEEPERS_MAX];
struct TimerEntry_t periodics[PERIODICS_MAX];

/** Kernel implementation of usleep() syscall */
int os_usleep(unsigned microseconds)
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id == 0xFF)
		{
			uint32_t microtime = os_get_micro_time();
			sleepers[q].thread_id = os_get_current_thread();
			ASSERT(sleepers[q].thread_id < OS_THREADS);
			sleepers[q].sleep_to = microtime + microseconds;
			os_thread_stop(os_get_current_thread());
			return 0;
		}
	}
	return E_NOTAVAIL;
}

/** Kernel implementation of setitimer() syscall */
int os_setitimer(unsigned microseconds)
{
	for (int q = 0; q < PERIODICS_MAX; ++q)
	{
		if (periodics[q].thread_id == 0xFF)
		{
			periodics[q].thread_id = os_get_current_thread();
			periodics[q].interval = microseconds;
			periodics[q].offset = os_get_micro_time() % microseconds;
			return 0;
		}
	}
	return E_NOTAVAIL;
}

void os_timer_init()
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		sleepers[q].thread_id = 0xFF;
	}

	for (int q = 0; q < PERIODICS_MAX; ++q)
	{
		periodics[q].thread_id = 0xFF;
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
	bool rv = 0;

	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		if (sleepers[q].thread_id != 0xFF)
		{
			uint32_t delay;
			if (sleepers[q].sleep_to < microtime)
			{
				delay = ~0 - (microtime - sleepers[q].sleep_to);
			}
			else
			{
				delay = sleepers[q].sleep_to - microtime;
			}
			if (delay < min_delay)
			{
				min_delay = delay;
				rv = true;
			}
		}
	}

	for (int q = 0; q < PERIODICS_MAX; ++q)
	{
		if (periodics[q].thread_id != 0xFF)
		{
			uint32_t delay;
			uint32_t in_interval = microtime % periodics[q].interval;

			if (in_interval > periodics[q].offset)
			{
				delay = periodics[q].interval - (in_interval - periodics[q].offset);
			}
			else
			{
				delay = periodics[q].offset - in_interval;
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
			if (sleepers[q].sleep_to == microtime)
			{
				// restart usleep-ed thread
				os_thread_continue(sleepers[q].thread_id);
				sleepers[q].thread_id = 0xFF;
			}
		}
	}

	for (int q = 0; q < PERIODICS_MAX; ++q)
	{
		if (periodics[q].thread_id != 0xFF)
		{
			uint32_t in_interval = microtime % periodics[q].interval;

			if (in_interval == periodics[q].offset)
			{
				os_kill(periodics[q].thread_id, SIGALARM);
			}
		}
	}
}
