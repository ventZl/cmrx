#include <cmrx/application.h>
#include <cmrx/ipc/timer.h>
#include <debug.h>

int init_high_prio(void * data)
{
    (void) data;
    usleep(500000);
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(syscall_usleep_init, 0x40000000, 0x60000000);
OS_APPLICATION(syscall_usleep_init);
OS_THREAD_CREATE(syscall_usleep_init, init_high_prio, NULL, 2);
