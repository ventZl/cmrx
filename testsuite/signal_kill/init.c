#include <cmrx/ipc/thread.h>
#include <cmrx/application.h>
#include <debug.h>
#include <utils.h>
#include <cmrx/ipc/signal.h>

void signal_handler(uint32_t signo)
{
    (void) signo;
    TEST_SUCCESS();
}

int thread_main(void *)
{
    signal(SIGALRM, signal_handler);
    kill(get_tid(), SIGSTOP);
    TEST_FAIL();
    return 0;
}

int init_main(void *)
{
    int thread_id = thread_create(thread_main, NULL, 32);
    sched_yield();
    kill(thread_id, SIGALRM);
    TEST_FAIL();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(signal_kill_init, 0x40000000, 0x60000000);
OS_APPLICATION(signal_kill_init);
OS_THREAD_CREATE(signal_kill_init, init_main, NULL, 64);
