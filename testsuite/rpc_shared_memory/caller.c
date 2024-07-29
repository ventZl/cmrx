#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <cmrx/ipc/rpc.h>
#include <cmrx/ipc/shmem.h>
#include <debug.h>
#include <utils.h>
#include "service.h"

char SHARED buffer[] = "Babedeadbabedead";

int caller_high_prio(void * data)
{
    (void) data;

    rpc_call(&service, method, sizeof(buffer), buffer);
	TEST_FAIL();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(rpc_shared_memory_caller, 0x40000000, 0x60000000);
OS_APPLICATION(rpc_shared_memory_caller);
OS_THREAD_CREATE(rpc_shared_memory_caller, caller_high_prio, NULL, 2);

