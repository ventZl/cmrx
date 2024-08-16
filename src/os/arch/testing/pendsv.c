#include <cmrx/os/context.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmrx/os/runtime.h>

bool schedule_context_switch_called = false;

void os_request_context_switch()
{
    schedule_context_switch_called = true;
//    ctxt_switch_pending = false;
}

