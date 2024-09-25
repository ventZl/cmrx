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
    ASSERT_EQUAL(os_threads[core[0].thread_next].state, THREAD_STATE_RUNNING);

    schedule_context_switch_called = true;
//    ctxt_switch_pending = false;
}

