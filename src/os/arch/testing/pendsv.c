#include <cmrx/os/context.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmrx/os/runtime.h>
#include <arch/corelocal.h>

#include <ctest.h>

bool schedule_context_switch_called = false;

extern struct OS_core_state_t core[OS_NUM_CORES];

void os_request_context_switch()
{
    Thread_t thread_current = core[0].thread_current;
    Thread_t thread_next = core[0].thread_next;

    // This was here. Yet, there are states where this rule doesn't hold
    // like if you call wait_for_object()
    // ASSERT_EQUAL(os_threads[thread_next].state, THREAD_STATE_READY);
    core[coreid()].thread_current = core[coreid()].thread_next;
    if (os_threads[thread_current].state == THREAD_STATE_RUNNING)
    {
        os_threads[thread_current].state = THREAD_STATE_READY;
    }
    os_threads[thread_next].state = THREAD_STATE_RUNNING;
    schedule_context_switch_called = true;
//    ctxt_switch_pending = false;
}

