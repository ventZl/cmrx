/** @addtogroup os_timer 
 * @{ 
 */
#include "timer.h"
#include "sched.h"
#include "signal.h"
#include "notify.h"
#include "txn.h"
#include <conf/kernel.h>

#include <stdint.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>

#include <stdbool.h>
#include <cmrx/clock.h>

/** List of all delays requested from kernel.
 * This structure contains all scheduled sleeps requested by all threads.
 * Every thread can technically request one periodic timer and one one-shot
 * delay.
 */
struct TimerEntry_t sleepers[SLEEPERS_MAX];

/** Determine if interval in sleep entry is periodic or not.
 * @returns 1 if interval is periodic, 0 if interval is one-shot.
 */
static bool is_periodic(struct TimerEntry_t * entry) {
    return entry->timer_type >= TIMER_PERIODIC;
}

/** Get how long the sleep should take.
 * Will clean the periodicity flag from interval.
 * @returns sleep time in microseconds.
 */
static unsigned get_sleeptime(const unsigned interval) {
    return interval;
}

/** Perform heavy lifting of setting timers
 * This routine will store timed event into list of timed events. If 
 * event is not periodic, it will also stop thread (\ref os_usleep semantics).
 *
 * @param txn transaction used to modify the table
 * @param slot number of slot in \ref sleepers list
 * @param interval amount of us for which thread should sleep
 * @param type type of timer to set up
 * @return 0 if timer was set up properly
 */
static int do_set_timed_event(Txn_t txn, unsigned slot, unsigned interval, enum eSleepType type)
{
    if (os_txn_commit(txn, TXN_READWRITE) == E_OK) {
        Thread_t thread_id = os_get_current_thread();
        uint32_t microtime = os_get_micro_time();
        struct TimerEntry_t * sleeper = &sleepers[slot];
        sleeper->thread_id = thread_id;
        ASSERT(sleeper->thread_id < OS_THREADS);
        sleeper->sleep_from = microtime;
        sleeper->interval = interval;
		sleeper->timer_type = type;
        os_txn_done();

		switch (type) {
			case TIMER_SLEEP:
				os_thread_stop(thread_id);
				break;

			default:
				break;
		}
		return 0;
    } else {
        return 1;
    }
}


int os_set_timed_event(unsigned microseconds, enum eSleepType type)
{
    Txn_t txn = os_txn_start();
	Thread_t thread_id = os_get_current_thread();

	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
        struct TimerEntry_t * sleeper = &sleepers[q];
		if (sleeper->thread_id == 0xFF)
		{
			if (do_set_timed_event(txn, q, microseconds, type) == 0) {
                return 0;
            } else {
                // Transaction has been aborted, restart search
                txn = os_txn_start();
                q = -1;
                continue;
            }
		}
		else
		{
			if (sleepers->thread_id == thread_id)
			{
				/* We are searching for interval timer / delay for this thread
				 * we found some entry belonging to this thread, but it is of
				 * different kind than we are searching for. It is delay and
				 * we are searching for interval timer or vice versa.
				 */
				if (type == sleeper->timer_type)
				{
					if (do_set_timed_event(txn, q, microseconds, type) == 0) {
                        return 0;
                    } else {
                        txn = os_txn_start();
                        q = -1;
                        continue;
                    }
				}
			}
		}
	}
	return E_NOTAVAIL;
}

int os_find_timer(Thread_t owner, enum eSleepType type)
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		struct TimerEntry_t * sleeper = &sleepers[q];
		if (sleeper->thread_id == owner)
		{
			if (type == sleeper->timer_type)
			{
				return q;
			}
		}
	}
	return TIMER_INVALID_ID;
}


int os_cancel_timed_event(Thread_t owner, enum eSleepType type)
{
	Txn_t txn;
	int timer_id = 0xFF;
	int rv;
	do {
		txn = os_txn_start();
		timer_id = os_find_timer(owner, type);
		if (timer_id == TIMER_INVALID_ID)
		{
			return E_NOTAVAIL;
		}
	} while (os_txn_commit(txn, TXN_READWRITE) != E_OK);

	rv = os_cancel_sleeper(timer_id);
	os_txn_done();

	return rv;
}

int os_cancel_sleeper(int sleeper_id)
{
	if (sleeper_id >= SLEEPERS_MAX)
	{
		return E_INVALID;
	}

	struct TimerEntry_t * sleeper = &sleepers[sleeper_id];
	sleeper->thread_id = 0xFF;

	return 0;
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
	return os_set_timed_event(microseconds, TIMER_SLEEP);
}

int os_setitimer(unsigned microseconds)
{
	if (microseconds > 0)
	{
		return os_set_timed_event(microseconds, TIMER_INTERVAL);
	}
	else
	{
		return os_cancel_timed_event(os_get_current_thread(), TIMER_INTERVAL);
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
        struct TimerEntry_t * sleeper = &sleepers[q];
		if (sleeper->thread_id != 0xFF)
		{
			/* Figure how long this particular sleeper is already sleeing */
			uint32_t sleeping = get_sleep_time(sleeper->sleep_from, microtime);
			/* Figure out how long should this particular sleeper continue
			 * to sleep
			 */
			uint32_t tosleep = get_sleeptime(sleeper->interval);

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
	bool yield_needed = false;
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		Txn_t txn = os_txn_start();

        struct TimerEntry_t * sleeper = &sleepers[q];
		if (sleeper->thread_id != 0xFF)
		{
            const unsigned sleeper_sleeptime = get_sleeptime(sleeper->interval);
			if (get_sleep_time(sleeper->sleep_from, microtime) >= sleeper_sleeptime)
			{
				if (os_txn_commit(txn, TXN_READWRITE) == E_OK)
				{
					switch (sleeper->timer_type) {
						case TIMER_SLEEP:
						case TIMER_INTERVAL:
							{
								struct OS_thread_t * thread = os_thread_by_id(sleeper->thread_id);
								os_thread_set_ready(thread);
								yield_needed = true;
								break;
							}

						case TIMER_TIMEOUT:
							os_notify_thread(sleeper->thread_id, q, EVT_TIMEOUT);
							break;

						default:
							break;
					}

					if (is_periodic(sleeper))
					{
						sleeper->sleep_from = sleeper->sleep_from + sleeper_sleeptime;
					}
					else
					{
						sleeper->thread_id = 0xFF;
					}
					os_txn_done();
				}
				else
				{
					// Restart the current iteration
					q = q - 1;
					continue;
				}
			}
		}
	}
	if (yield_needed)
	{
		os_sched_yield();
	}
}

uint32_t os_cpu_freq_get(void)
{
	return timing_get_current_cpu_freq();
}

/** @} */
