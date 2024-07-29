#include "cmrx/os/runtime.h"
#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>
#include <utils.h>

int new_thread_entry(void * data)
{
    (void) data;
//    TEST_SUCCESS();
    return 0;
}

int init_main(void * data)
{
    (void) data;
    uint8_t thread_id = thread_create(new_thread_entry, NULL, 32);
	TEST_STEP(1);
    sched_yield();
    (void) thread_join(thread_id);
    TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(thread_join_init, 0x40000000, 0x60000000);
OS_APPLICATION(thread_join_init);
OS_THREAD_CREATE(thread_join_init, init_main, NULL, 64);

