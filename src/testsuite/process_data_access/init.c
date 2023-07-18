#include <cmrx/application.h>
#include <debug.h>

static int process_variable = 0;

int init_main(void * data)
{
    process_variable = (int) data;
	TEST_SUCCESS();
	return 0;
}

OS_APPLICATION_MMIO_RANGE(process_data_access_init, 0x40000000, 0x60000000);
OS_APPLICATION(process_data_access_init);
OS_THREAD_CREATE(process_data_access_init, init_main, NULL, 64);

