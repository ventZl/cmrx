#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>
#include <utils.h>

int new_thread_entry(void * data)
{
    (void) data;
    TEST_SUCCESS();
    while (1) {
        // make infinite cycle here
    }
    return 0;
}

int init_main(void * data)
{
    (void) data;
    uint8_t thread_id = thread_create(new_thread_entry, NULL, 66);
    (void) thread_id;
    sched_yield();
	TEST_STEP(1);
    setpriority(128);
	return 0;
}

OS_APPLICATION_MMIO_RANGE(setpriority_init, 0x40000000, 0x60000000);
OS_APPLICATION(setpriority_init);
OS_THREAD_CREATE(setpriority_init, init_main, NULL, 64);

