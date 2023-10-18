#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>

int new_thread_entry(void * data)
{
    TEST_SUCCESS();
    return 0;
}

int init_main(void * data)
{
    uint8_t thread_id = thread_create(new_thread_entry, NULL, 32);
	TEST_STEP(1);
    sched_yield();
    int rv;
    thread_join(thread_id, &rv);
    TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(setpriority_init, 0x40000000, 0x60000000);
OS_APPLICATION(setpriority_init);
OS_THREAD_CREATE(setpriority_init, init_main, NULL, 64);
