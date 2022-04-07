#include <cmrx/application.h>
#include "debug.h"

int init_main(void * data)
{
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(test_init, 0x40000000, 0x60000000);
OS_APPLICATION(test_init);
OS_THREAD_CREATE(test_init, init_main, NULL, 64);
