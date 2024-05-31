#include <cmrx/os/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];

extern int os_stack_create();
extern int os_thread_exit(int status);
extern int os_thread_kill(uint8_t thread_id, int status);
extern bool schedule_context_switch_called;

CTEST_DATA(os_thread_kill) {
};

CTEST_SETUP(os_thread_kill) {
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

CTEST2(os_thread_kill, ready_thread) {
    (void) data;

    int rv = os_thread_kill(1, 54);
    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_FINISHED);
    ASSERT_EQUAL(os_threads[1].exit_status, 54);
    ASSERT_EQUAL(os_stacks.allocations, 0x1);
}

CTEST2(os_thread_kill, current_thread) {
    (void) data;

    int rv = os_thread_kill(0, 54);
    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_FINISHED);
    ASSERT_EQUAL(os_threads[0].exit_status, 54);
    ASSERT_EQUAL(os_stacks.allocations, 0x2);
}

CTEST2(os_thread_kill, finished_thread) {
    (void) data;

    int rv = os_thread_kill(3, 54);
    ASSERT_EQUAL(rv, E_INVALID);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(os_threads[0].exit_status, 0);
    ASSERT_EQUAL(os_stacks.allocations, 0x3);
}

CTEST2(os_thread_kill, out_of_range_thread) {
    (void) data;

    int rv = os_thread_kill(254, 54);
    ASSERT_EQUAL(rv, E_INVALID);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(os_threads[0].exit_status, 0);
    ASSERT_EQUAL(os_stacks.allocations, 0x3);
}

CTEST2(os_thread_kill, exit_current_thread) {
    (void) data;

    int rv = os_thread_exit(54);
    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_FINISHED);
    ASSERT_EQUAL(os_threads[0].exit_status, 54);
    ASSERT_EQUAL(os_stacks.allocations, 0x2);
}


