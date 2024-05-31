#include <cmrx/os/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];

extern int os_stack_create();
extern int os_thread_exit(int status);
extern int os_thread_stop(uint8_t thread_id);
extern bool schedule_context_switch_called;

CTEST_DATA(os_thread_stop) {
};

CTEST_SETUP(os_thread_stop) {
    (void) data;
    memset(&os_stacks, 0, sizeof(os_stacks));
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].priority = 32;
    os_threads[1].priority = 32;
    os_threads[2].priority = 16;
    os_threads[3].priority = 64;

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_READY;
    os_threads[2].state = THREAD_STATE_EMPTY;
    os_threads[3].state = THREAD_STATE_FINISHED;

    os_threads[0].stack_id = os_stack_create();
    os_threads[1].stack_id = os_stack_create();

    schedule_context_switch_called = false;
    core[0].thread_current = 0;
}

CTEST2(os_thread_stop, ready_thread) {
    (void) data;

    int rv = os_thread_stop(1);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_STOPPED);
}

CTEST2(os_thread_stop, running_thread) {
    (void) data;

    int rv = os_thread_stop(0);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_STOPPED);
}

CTEST2(os_thread_stop, finished_thread) {
    (void) data;

    int rv = os_thread_stop(3);

    ASSERT_EQUAL(rv, E_NOTAVAIL);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
}

CTEST2(os_thread_stop, out_of_range_thread) {
    (void) data;

    int rv = os_thread_stop(254);

    ASSERT_EQUAL(rv, E_INVALID);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
}

CTEST2(os_thread_stop, stop_continue) {
    (void) data;

    int rv = os_thread_stop(1);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_STOPPED);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(core[0].thread_current, 0);

    rv = os_thread_continue(1);

    // Expect round robin scheduling being applied to threads of
    // same priority
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_READY);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(core[0].thread_current, 1);

}
