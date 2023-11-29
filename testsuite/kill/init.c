#include <cmrx/ipc/thread.h>
#include <cmrx/application.h>
#include <debug.h>
#include <cmrx/ipc/signal.h>

int thread_main(void *)
{
    kill(get_tid(), SIGSTOP);
    TEST_FAIL();
    return 0;
}

int init_main(void *)
{
    thread_create(thread_main, NULL, 32);
    sched_yield();
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(kill_init, 0x40000000, 0x60000000);
OS_APPLICATION(kill_init);
OS_THREAD_CREATE(kill_init, init_main, NULL, 64);
