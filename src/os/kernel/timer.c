/** @addtogroup os_timer 
 * @{ 
 */
#include "timer.h"
#include "sched.h"
#include "signal.h"
#include "notify.h"
#include "txn.h"
#include "algo.h"
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
struct TimerQueueEntry_t sleeper_queue[SLEEPERS_MAX];
unsigned sleeper_queue_size;
unsigned sleeper_queue_cursor;

/** Determine if interval in sleep entry is periodic or not.
 * @returns 1 if interval is periodic, 0 if interval is one-shot.
 */
static bool is_periodic(struct TimerEntry_t * entry) {
    return entry->timer_type >= TIMER_PERIODIC;
}

int os_find_timer_slot(Thread_t owner, enum eSleepType type)
{
	const uint32_t sought_key = SLEEPER_KEY(type, owner);
	uint32_t offs = HASH_SEARCH(sleepers, key, sought_key, SLEEPERS_MAX);

	return offs;
}

int os_find_timer_queue(int timer_slot_id)
{
	if (timer_slot_id >= SLEEPERS_MAX)
	{
		return TIMER_INVALID_ID;
	}

	struct TimerEntry_t * sleeper = &sleepers[timer_slot_id];

	// Find timer will find a free slot if there is no match
	if (sleeper->key == HASH_EMPTY)
	{
		return TIMER_INVALID_ID;
	}
	unsigned next_resume = sleeper->sleep_from + sleeper->interval;
	unsigned queue_offs = BINARY_SEARCH(sleeper_queue, resume_time, next_resume, sleeper_queue_size);
	do {
		if (sleeper_queue[queue_offs].entry_id == timer_slot_id)
		{
			return queue_offs;
		}
	} while (sleeper_queue[queue_offs].resume_time == next_resume);

	return TIMER_INVALID_ID;
}

void os_dequeue_timed_event(unsigned queue_offs)
{
	// Here we delete the old queue entry as it points to wrong point in time
	ARRAY_DELETE(sleeper_queue, queue_offs, sleeper_queue_size);
	if (sleeper_queue_cursor > queue_offs)
	{
		sleeper_queue_cursor--;
	}
}

void os_enqueue_timed_event(unsigned sleeper_id, uint32_t next_resume)
{
	unsigned queue_offs = BINARY_SEARCH(sleeper_queue, resume_time, next_resume, sleeper_queue_size);
	ARRAY_INSERT(sleeper_queue, queue_offs, sleeper_queue_size);
	struct TimerQueueEntry_t * queue = &sleeper_queue[queue_offs];
	queue->resume_time = next_resume;
	queue->entry_id = sleeper_id;
	if (sleeper_queue_cursor > queue_offs)
	{
		sleeper_queue_cursor--;
	}
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
static int do_set_timed_event(Txn_t txn, const unsigned slot, const unsigned interval, const enum eSleepType type)
{
	unsigned old_queue_offs = os_find_timer_queue(slot);

    if (os_txn_commit(txn, TXN_READWRITE) == E_OK) {
		uint32_t microtime = os_get_micro_time();
		Thread_t thread_id = os_get_current_thread();
		// it is OK to overflow here
		uint32_t next_resume = microtime + interval;

        struct TimerEntry_t * sleeper = &sleepers[slot];
		sleeper->key = SLEEPER_KEY(type, thread_id);
        sleeper->thread_id = thread_id;
        ASSERT(sleeper->thread_id < OS_THREADS);
        sleeper->sleep_from = microtime;
        sleeper->interval = interval;
		sleeper->timer_type = type;

		if (sleeper_queue[old_queue_offs].resume_time != next_resume || old_queue_offs == TIMER_INVALID_ID)
		{
			if (old_queue_offs != TIMER_INVALID_ID)
			{
				os_dequeue_timed_event(old_queue_offs);
			}

			os_enqueue_timed_event(slot, next_resume);
		}

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

	do {
		uint32_t slot_id = os_find_timer_slot(thread_id, type);

		if (do_set_timed_event(txn, slot_id, microseconds, type) == 0)
		{
			return E_OK;
		}
		else
		{
			txn = os_txn_start();
			continue;
		}
	} while (true);

	return E_NOTAVAIL;
}

int os_find_timer(Thread_t owner, enum eSleepType type)
{
	int timer_id = os_find_timer_slot(owner, type);
	return os_find_timer_queue(timer_id);
}

int os_cancel_timed_event(Thread_t owner, enum eSleepType type)
{
	Txn_t txn;
	int timer_id = TIMER_INVALID_ID;
	int rv;
	do {
		txn = os_txn_start();
		timer_id = os_find_timer(owner, type);

	} while (os_txn_commit(txn, TXN_READWRITE) != E_OK);

	if (timer_id == TIMER_INVALID_ID)
	{
		rv = E_NOTAVAIL;
	}
	else
	{
		rv = os_cancel_sleeper(timer_id);
	}
	os_txn_done();

	return rv;
}

int os_cancel_sleeper(unsigned queue_id)
{
	if (queue_id >= sleeper_queue_size)
	{
		return E_INVALID;
	}

	struct TimerQueueEntry_t * queue_entry = &sleeper_queue[queue_id];
	ASSERT(queue_entry->entry_id < SLEEPERS_MAX);
	int sleeper_id = queue_entry->entry_id;

	os_dequeue_timed_event(queue_id);

	struct TimerEntry_t * sleeper = &sleepers[sleeper_id];
	ASSERT(sleeper->key != HASH_EMPTY);
	sleeper->key = HASH_EMPTY;

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

void os_timer_init(void)
{
	for (int q = 0; q < SLEEPERS_MAX; ++q)
	{
		sleepers[q].key = HASH_EMPTY;
	}

	sleeper_queue_size = 0;
	sleeper_queue_cursor = 0;
}

__attribute__((deprecated)) bool os_schedule_timer(unsigned * delay)
{
	unsigned microtime = os_get_micro_time();
	uint32_t min_delay = ~0;
	bool rv = false;

	if (sleeper_queue_size == 0)
	{
		return false;
	}

	unsigned slot_id = sleeper_queue_cursor;
	if (sleeper_queue_size > 1)
	{
		slot_id = (slot_id + 1) % sleeper_queue_size;
	}

	return sleeper_queue[slot_id].resume_time - microtime;
}

void os_sleeper_reschedule(unsigned sleeper_queue_id)
{
	struct TimerQueueEntry_t * queue_entry = &sleeper_queue[sleeper_queue_id];

	const uint32_t sleeper_id = queue_entry->entry_id;

	struct TimerEntry_t * sleeper_entry = &sleepers[sleeper_id];
	// Overflow is OK here
	uint32_t next_resume = queue_entry->resume_time + sleeper_entry->interval;

	os_dequeue_timed_event(sleeper_queue_id);
	os_enqueue_timed_event(sleeper_id, next_resume);
}

void os_run_timer(uint32_t start_microtime, uint32_t interval)
{
	bool run_timers = true;
	const uint32_t end_microtime = start_microtime + interval;
	bool wrapped = end_microtime < start_microtime;
	if (sleeper_queue_size == 0)
	{
		return;
	}

	Txn_t txn = os_txn_start();

	do {
		const struct TimerQueueEntry_t * queue_entry = &sleeper_queue[sleeper_queue_cursor];
		const uint32_t entry_resume = queue_entry->resume_time;
		bool timer_due = false;

		if (entry_resume <= end_microtime || (wrapped && entry_resume > start_microtime))
		{
			timer_due = true;
		}
		if (os_txn_commit(txn, TXN_READWRITE) == E_OK)
		{
			if (timer_due)
			{
				struct TimerEntry_t * sleeper = &sleepers[queue_entry->entry_id];
				ASSERT(sleeper_queue_cursor < sleeper_queue_size );
				ASSERT(queue_entry->entry_id < SLEEPERS_MAX);
				ASSERT(sleeper->key != HASH_EMPTY);
				switch (sleeper->timer_type) {
					case TIMER_SLEEP:
					case TIMER_INTERVAL:
					{
						struct OS_thread_t * thread = os_thread_by_id(sleeper->thread_id);
						os_thread_set_ready(thread);
						break;
					}

					case TIMER_TIMEOUT:
						os_notify_thread(sleeper->thread_id, sleeper_queue_cursor, EVT_TIMEOUT);
						break;

					default:
						break;
				}

				if (is_periodic(sleeper))
				{
					os_sleeper_reschedule(sleeper_queue_cursor);
				}
				else
				{
					os_cancel_sleeper(sleeper_queue_cursor);
				}

				if (sleeper_queue_size > 0)
				{
					sleeper_queue_cursor = (sleeper_queue_cursor + 1) % sleeper_queue_size;
				}
				else
				{
					run_timers = false;
				}
			}
			else
			{
				run_timers = false;
			}

			os_txn_done();
		}
		txn = os_txn_start();
	} while (run_timers);
}

uint32_t os_cpu_freq_get(void)
{
	return timing_get_current_cpu_freq();
}

/** @} */
