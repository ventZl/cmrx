#include <kernel/timer.h>
#include <kernel/runtime.h>
#include <ctest.h>
#include <arch/corelocal.h>
#include "test_traits.h"

// Kernel private functions and variables, not part of any header
extern bool timing_provider_delay_called;
extern long timing_provider_delay_us;
extern struct OS_core_state_t core[OS_NUM_CORES];
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];
extern struct OS_thread_t os_threads[OS_THREADS];
extern struct TimerQueueEntry_t sleeper_queue[SLEEPERS_MAX];
extern unsigned sleeper_queue_size;

CTEST_DATA(os_usleep) {
};

CTEST_SETUP(os_usleep) {
    timing_provider_delay_called = false;
    core[0].thread_current = 4;
    for (unsigned q = 0; q < OS_THREADS; ++q)
    {
        os_threads[q].state = q == core[0].thread_current ? THREAD_STATE_RUNNING : THREAD_STATE_READY;
        os_threads[q].priority = q == core[0].thread_current ? 32 : 64;
    }

    os_timer_init();
}

CTEST2(os_usleep, busy_wait) {
    int rv = os_usleep(100);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(timing_provider_delay_called, true);
    ASSERT_EQUAL(timing_provider_delay_us, 100);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_RUNNING);
}

/* Check that sleeping will create timer event with proper settings */
CTEST2(os_usleep, scheduled_event) {
    int rv = os_usleep(10000);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].interval, 10000);
    ASSERT_EQUAL(sleepers[slot_id].timer_type, TIMER_SLEEP);
}

/* Check that rescheduling sleep event to different time will update entry without creating new one */
CTEST2(os_usleep, reschedule_event) {
    int rv = os_usleep(10000);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].interval, 10000);

    // os_usleep suspended this thread, make it running again
    // so we can simulate resetting schedule for os_usleep for this thread
    core[0].thread_current = 4;
    os_threads[4].state = THREAD_STATE_RUNNING;

    rv = os_usleep(9876);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].interval, 9876);

    // This should end up as a busywait, no change to
    // sleeper table.

    core[0].thread_current = 4;
    os_threads[4].state = THREAD_STATE_RUNNING;

    rv = os_usleep(150);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(timing_provider_delay_called, true);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].interval, 9876);
}

/* Check that scheduling sleep from multiple threads will create multiple entries */
CTEST2(os_usleep, schedule_multiple_threads) {
    int rv = os_usleep(10000);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);

    core[0].thread_current = 3;
    os_threads[3].state = THREAD_STATE_RUNNING;
    rv = os_usleep(12000);
    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(sleeper_queue_size, 2);

    unsigned second_slot_id = sleeper_queue[0].entry_id;
    // Check that first entry is still the same
    ASSERT_EQUAL(slot_id, second_slot_id);

    second_slot_id = sleeper_queue[1].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[second_slot_id].thread_id, 3);
}

CTEST2(os_usleep, cancel_multiple_threads) {
    int rv = os_usleep(10000);
    ASSERT_EQUAL(rv, E_OK);

    core[0].thread_current = 3;
    os_threads[3].state = THREAD_STATE_RUNNING;
    rv = os_usleep(10000);
    ASSERT_EQUAL(rv, E_OK);

    core[0].thread_current = 4;
}

CTEST2(os_usleep, resume_suspended_thread) {
    int rv = os_usleep(10000);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(thread_state(4), THREAD_STATE_STOPPED);

    os_run_timer(0, 10000);
    os_sched_yield();
    ASSERT_EQUAL(thread_state(4), THREAD_STATE_RUNNING);
}
