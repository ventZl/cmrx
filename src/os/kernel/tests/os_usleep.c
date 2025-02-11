#include <kernel/timer.h>
#include <kernel/runtime.h>
#include <ctest.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern bool timing_provider_delay_called;
extern long timing_provider_delay_us;
extern struct OS_core_state_t core[OS_NUM_CORES];
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];
extern struct OS_thread_t os_threads[OS_THREADS];

CTEST_DATA(os_usleep) {
};

CTEST_SETUP(os_usleep) {
    timing_provider_delay_called = false;
    core[0].thread_current = 4;
    for (unsigned q = 0; q < OS_THREADS; ++q)
    {
        os_threads[q].state = q == core[0].thread_current ? THREAD_STATE_RUNNING : THREAD_STATE_READY;
    }

    os_timer_init();
}

CTEST2(os_usleep, busy_wait) {
    int rv = os_usleep(100);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, true);
    ASSERT_EQUAL(timing_provider_delay_us, 100);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_RUNNING);
}

CTEST2(os_usleep, scheduled_event) {
    int rv = os_usleep(10000);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);
    ASSERT_EQUAL(sleepers[0].interval, 10000);
}

CTEST2(os_usleep, reschedule_event) {
    int rv = os_usleep(10000);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);
    ASSERT_EQUAL(sleepers[0].interval, 10000);

    // os_usleep suspended this thread, make it running again
    // so we can simulate resetting schedule for os_usleep for this thread
    core[0].thread_current = 4;
    os_threads[4].state = THREAD_STATE_RUNNING;

    rv = os_usleep(9876);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);
    ASSERT_EQUAL(sleepers[0].interval, 9876);

    // This should end up as a busywait, no change to
    // sleeper table.

    core[0].thread_current = 4;
    os_threads[4].state = THREAD_STATE_RUNNING;

    rv = os_usleep(150);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, true);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);
    ASSERT_EQUAL(sleepers[0].interval, 9876);
}

CTEST2(os_usleep, schedule_multiple_threads) {
    int rv = os_usleep(10000);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);

    core[0].thread_current = 3;
    os_threads[3].state = THREAD_STATE_RUNNING;
    rv = os_usleep(10000);
    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);
    ASSERT_EQUAL(sleepers[1].thread_id, 3);
}

CTEST2(os_usleep, cancel_multiple_threads) {
    int rv = os_usleep(10000);
    ASSERT_EQUAL(rv, 0);

    core[0].thread_current = 3;
    os_threads[3].state = THREAD_STATE_RUNNING;
    rv = os_usleep(10000);
    ASSERT_EQUAL(rv, 0);

    core[0].thread_current = 4;
}
