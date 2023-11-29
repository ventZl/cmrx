#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>

int new_thread_entry(void * data)
{
    (void) data;
    thread_exit(42);
    // this should never execute
    TEST_FAIL();
    return 0;
}

int init_main(void * data)
{
    (void) data;
    uint8_t thread_id = thread_create(new_thread_entry, NULL, 32);
	TEST_STEP(1);
    sched_yield();
    int rv = thread_join(thread_id);
    if (rv == 42)
    {
        TEST_SUCCESS();
    }
    else 
    {
        TEST_FAIL();
    }
	return 0;
}

OS_APPLICATION_MMIO_RANGE(thread_exit_init, 0x40000000, 0x60000000);
OS_APPLICATION(thread_exit_init);
OS_THREAD_CREATE(thread_exit_init, init_main, NULL, 64);

