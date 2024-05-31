#include <cmrx/os/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];

extern int os_stack_create();
extern bool schedule_context_switch_called;

CTEST_DATA(os_thread_continue) {
};

CTEST_SETUP(os_thread_continue) {
    (void) data;
    memset(&os_stacks, 0, sizeof(os_stacks));
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].priority = 32;
    os_threads[1].priority = 32;
    os_threads[2].priority = 16;
    os_threads[3].priority = 64;
    os_threads[4].priority = 48;
    os_threads[5].priority = 16;
    os_threads[6].priority = 96;

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_STOPPED;
    os_threads[2].state = THREAD_STATE_EMPTY;
    os_threads[3].state = THREAD_STATE_FINISHED;
    os_threads[4].state = THREAD_STATE_STOPPED;
    os_threads[5].state = THREAD_STATE_STOPPED;
    os_threads[6].state = THREAD_STATE_READY;

    os_threads[0].stack_id = os_stack_create();
    os_threads[1].stack_id = os_stack_create();
    os_threads[4].stack_id = os_stack_create();
    os_threads[5].stack_id = os_stack_create();
    os_threads[6].stack_id = os_stack_create();

    schedule_context_switch_called = false;
    core[0].thread_current = 0;
}

CTEST2(os_thread_continue, not_stopped_thread) {
    (void) data;

    int rv = os_thread_continue(0);

    ASSERT_EQUAL(rv, E_NOTAVAIL);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);

    rv = os_thread_continue(6);

    ASSERT_EQUAL(rv, E_NOTAVAIL);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[6].state, THREAD_STATE_READY);
}

CTEST2(os_thread_continue, same_priority) {
    (void) data;

    int rv = os_thread_continue(1);

    ASSERT_EQUAL(rv, 0);
    // Expect round robin scheduling being applied to threads of
    // same priority
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(core[0].thread_current, 1);
    ASSERT_EQUAL(core[0].thread_prev, 0);
}

CTEST2(os_thread_continue, lower_priority) {
    (void) data;

    int rv = os_thread_continue(4);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(core[0].thread_current, 0);
}

CTEST2(os_thread_continue, higher_priority) {
    (void) data;

    int rv = os_thread_continue(5);

    ASSERT_EQUAL(rv, 0);
    // Expect round robin scheduling being applied to threads of
    // same priority
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(core[0].thread_current, 5);
    ASSERT_EQUAL(core[0].thread_prev, 0);
}

CTEST2(os_thread_continue, out_of_range_thread) {
    (void) data;

    int rv = os_thread_continue(254);

    ASSERT_EQUAL(rv, E_INVALID);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(core[0].thread_current, 0);
}

CTEST2(os_thread_continue, continue_stop) {
    (void) data;

    int rv = os_thread_continue(4);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_READY);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(core[0].thread_current, 0);

    rv = os_thread_stop(4);

    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_STOPPED);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(core[0].thread_current, 0);
}
