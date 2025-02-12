#include <kernel/sched.h>
#include <kernel/timer.h>
#include <kernel/signal.h>
#include <cmrx/ipc/signal.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];

CTEST_DATA(os_sched_timing_callback) {
};

CTEST_SETUP(os_sched_timing_callback) {
    memset(&os_threads, 0, sizeof(os_threads));
    core[0].thread_current = 0;
    memset(&sleepers, 0xFF, sizeof(sleepers));
}

CTEST2(os_sched_timing_callback, round_robin_same_prio) {
    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_READY;

    os_threads[0].priority = 32;
    os_threads[1].priority = 32;

    os_sched_timing_callback(25);

    ASSERT_EQUAL(core[0].thread_current, 1);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_READY);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_RUNNING);

    os_sched_timing_callback(25);

    ASSERT_EQUAL(core[0].thread_current, 0);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_READY);
}

CTEST2(os_sched_timing_callback, one_thread) {
    os_threads[0].state = THREAD_STATE_RUNNING;

    os_threads[0].priority = 32;

    os_sched_timing_callback(25);

    ASSERT_EQUAL(core[0].thread_current, 0);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
}

CTEST2(os_sched_timing_callback, set_itimer) {
    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_READY;

    os_threads[0].priority = 32;
    os_threads[1].priority = 48;

    // Nothing happens
    os_sched_timing_callback(2500);

    ASSERT_EQUAL(core[0].thread_current, 0);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);

    int tim_ret = os_setitimer(2500);

    ASSERT_EQUAL(tim_ret, 0);

    int thr_ret = os_kill(0, SIGSTOP);
    ASSERT_EQUAL(thr_ret, 0);

    os_sched_timing_callback(2400);
    ASSERT_EQUAL(core[0].thread_current, 1);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_RUNNING);

    os_sched_timing_callback(200);
    ASSERT_EQUAL(core[0].thread_current, 0);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);

    thr_ret = os_kill(0, SIGSTOP);
    ASSERT_EQUAL(thr_ret, 0);

    os_sched_timing_callback(2300);
    ASSERT_EQUAL(core[0].thread_current, 1);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_RUNNING);

    os_sched_timing_callback(200);
    ASSERT_EQUAL(core[0].thread_current, 0);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);

}
