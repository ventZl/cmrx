#include <cmrx/os/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];

extern int os_stack_create();
extern bool schedule_context_switch_called;

CTEST_DATA(os_setpriority) {
};

CTEST_SETUP(os_setpriority) {
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

CTEST2(os_setpriority, decrease_priority) {
    (void) data;

    int rv = os_setpriority(48);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[0].priority, 48);
    ASSERT_EQUAL(core[0].thread_current, 0);

    rv = os_setpriority(128);
    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(os_threads[0].priority, 128);
    ASSERT_EQUAL(core[0].thread_current, 6);

}
