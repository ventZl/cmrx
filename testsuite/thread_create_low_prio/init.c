#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>

int new_thread_entry(void * data)
{
    TEST_FAIL();
    return 0;
}

int init_main(void * data)
{
    thread_create(new_thread_entry, NULL, 128);
    sched_yield();
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(thread_create_low_prio_init, 0x40000000, 0x60000000);
OS_APPLICATION(thread_create_low_prio_init);
OS_THREAD_CREATE(thread_create_low_prio_init, init_main, NULL, 64);

