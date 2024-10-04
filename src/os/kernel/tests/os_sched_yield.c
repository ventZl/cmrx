#include <kernel/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

extern struct OS_core_state_t core[OS_NUM_CORES];
extern bool schedule_context_switch_called;

void sim_switch_threads()
{
    core[0].thread_current = core[0].thread_next;
}

CTEST(os_sched_yield, same_prio) {
    for (int q = 0; q < 2; ++q)
    {
        os_threads[q].priority = 32;
    }
    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_READY;

    core[0].thread_current = 0;
    os_sched_yield();

    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(core[0].thread_current, 1);
}

CTEST(os_sched_yield, higher_prio) {
    os_threads[0].priority = 32;
    os_threads[1].priority = 16;

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_READY;

    core[0].thread_current = 0;
    os_sched_yield();

    ASSERT_EQUAL(schedule_context_switch_called, true);
    ASSERT_EQUAL(core[0].thread_current, 1);

    schedule_context_switch_called = false;

    // run sched_yield again. This time, context switch
    // should not be scheduled, because there is no better
    // thread to schedule.
    os_sched_yield();

    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(core[0].thread_current, 1);
}
