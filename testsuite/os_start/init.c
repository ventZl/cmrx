#include <cmrx/application.h>
#include <debug.h>
#include <utils.h>

int init_main(void * data)
{
    (void) data;
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(os_start_init, 0x40000000, 0x60000000);
OS_APPLICATION(os_start_init);
OS_THREAD_CREATE(os_start_init, init_main, NULL, 64);
