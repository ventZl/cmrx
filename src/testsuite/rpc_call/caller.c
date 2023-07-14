#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <cmrx/ipc/rpc.h>
#include <debug.h>
#include "service.h"

#define VA_COUNT(service, method, ...) RPC_GET_ARG_COUNT(__VA_ARGS__)

int variable = VA_COUNT(service_ptr, method, 1);

int caller_high_prio(void * data)
{
    (void) data;

    rpc_call(service_ptr, method, 1);
	TEST_FAIL();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(rpc_call_caller, 0x40000000, 0x60000000);
OS_APPLICATION(rpc_call_caller);
OS_THREAD_CREATE(rpc_call_caller, caller_high_prio, NULL, 2);

