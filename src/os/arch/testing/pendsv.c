#include <kernel/context.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/runtime.h>
#include <arch/corelocal.h>
#include <kernel/arch/context.h>
#include <ctest.h>

bool schedule_context_switch_called = false;
// If set to false, then schedule_context_switch won't perform the thread switch
bool schedule_context_switch_perform_switch = true;
void (*schedule_context_switch_callback)() = NULL;

extern struct OS_core_state_t core[OS_NUM_CORES];

void os_request_context_switch(bool activate)
{

    if (schedule_context_switch_perform_switch)
    {
        if (activate)
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

            // Currently existing tests expect this to be called only
            // for activate = true cases.
            schedule_context_switch_called = true;
        }
    }


    if (schedule_context_switch_callback != NULL)
    {
        schedule_context_switch_callback();
    }
//    ctxt_switch_pending = false;
}
