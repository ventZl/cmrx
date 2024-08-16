#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>
#include <utils.h>

int init_high_prio(void * data)
{
    (void) data;
    sched_yield();
	TEST_SUCCESS();
	return 0;
}

int init_low_prio(void * data)
{
    (void) data;
    TEST_FAIL();
    return 0;
}

OS_APPLICATION_MMIO_RANGE(sched_yield_init, 0x40000000, 0x60000000);
OS_APPLICATION(sched_yield_init);
OS_THREAD_CREATE(sched_yield_init, init_high_prio, NULL, 2);
OS_THREAD_CREATE(sched_yield_init, init_low_prio, NULL, 64);
