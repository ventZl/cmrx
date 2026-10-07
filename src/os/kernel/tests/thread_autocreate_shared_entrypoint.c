#include <cmrx/application.h>

static int thread_data_1;
static int thread_data_2;

static int worker(void * data)
{
    return data != NULL;
}

OS_APPLICATION_MMIO_RANGE(thread_autocreate_shared_entrypoint, 0, 0);
OS_APPLICATION(thread_autocreate_shared_entrypoint);
OS_THREAD_CREATE(thread_autocreate_shared_entrypoint, worker, &thread_data_1, 2);
OS_THREAD_CREATE(thread_autocreate_shared_entrypoint, worker, &thread_data_2, 64);
