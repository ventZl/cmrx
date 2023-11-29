#include <cmrx/application.h>
#include <cmrx/ipc/rpc.h>
#include <debug.h>
#include "service.h"

int caller_high_prio(void * data)
{
    (void) data;

    rpc_call(&service, method, 1);
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(rpc_return_caller, 0x40000000, 0x60000000);
OS_APPLICATION(rpc_return_caller);
OS_THREAD_CREATE(rpc_return_caller, caller_high_prio, NULL, 2);

