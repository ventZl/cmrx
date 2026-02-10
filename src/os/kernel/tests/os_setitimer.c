#include <kernel/timer.h>
#include <kernel/runtime.h>
#include <kernel/algo.h>
#include <ctest.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern bool timing_provider_delay_called;
extern long timing_provider_delay_us;
extern struct OS_core_state_t core[OS_NUM_CORES];
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];
extern struct TimerQueueEntry_t sleeper_queue[SLEEPERS_MAX];
extern unsigned sleeper_queue_size;

CTEST_DATA(os_setitimer) {
};

CTEST_SETUP(os_setitimer) {
    timing_provider_delay_called = false;
    core[0].thread_current = 4;
    for (unsigned q = 0; q < OS_THREADS; ++q)
    {
        os_threads[q].state = q == core[0].thread_current ? THREAD_STATE_RUNNING : THREAD_STATE_READY;
    }

    os_timer_init();
}

/* Set interval timer */
CTEST2(os_setitimer, add_interval_timer) {
    int rv = os_setitimer(10000);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].key, SLEEPER_KEY(TIMER_PERIODIC, 4));
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].interval, 10000U);
    ASSERT_EQUAL(sleepers[slot_id].timer_type, TIMER_PERIODIC);
}

CTEST2(os_setitimer, cancel_interval_timer) {
    int rv = os_setitimer(10000);
    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);

    rv = os_setitimer(0);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(sleeper_queue_size, 0);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 0xFF);
    ASSERT_EQUAL(sleepers[slot_id].key, HASH_EMPTY);
}

CTEST2(os_setitimer, update_interval_timer) {
    int rv = os_setitimer(10000);
    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);

    rv = os_setitimer(12000);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[slot_id].key, SLEEPER_KEY(TIMER_PERIODIC, 4));
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
}

/* Check that interval timer and sleep will create two independent timer entries */
CTEST2(os_setitimer, itimer_and_sleep) {
    int rv = os_setitimer(10000);
    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(sleeper_queue_size, 1);
    unsigned slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].timer_type, TIMER_PERIODIC);

    rv = os_usleep(5000);
    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(sleeper_queue_size, 2);
    slot_id = sleeper_queue[0].entry_id;
    ASSERT_EQUAL(sleepers[slot_id].thread_id, 4);
    ASSERT_EQUAL(sleepers[slot_id].timer_type, TIMER_SLEEP);
}
