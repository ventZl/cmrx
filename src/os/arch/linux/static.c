#include <cmrx/sys/runtime.h>
#include <kernel/arch/static.h>
#include <conf/kernel.h>
#include <stdbool.h>
#include <string.h>

static unsigned process_count = 0;
static unsigned thread_count = 0;

static struct OS_process_definition_t os_process_definitions[OS_PROCESSES];
static struct OS_thread_create_t os_thread_definitions[OS_THREADS];


void cmrx_posix_register_application(const struct OS_process_definition_t * process)
{
    if (process_count == 0)
    {
        memset(&os_process_definitions, 0, sizeof(os_process_definitions));
    }

    memcpy(&os_process_definitions[process_count], process, sizeof(struct OS_process_definition_t));
    process_count++;
}

void cmrx_posix_register_thread(const struct OS_thread_create_t * thread)
{
    if (thread_count == 0)
    {
        memset(&os_thread_definitions, 0, sizeof(os_thread_definitions));
    }

    memcpy(&os_thread_definitions[thread_count], thread, sizeof(struct OS_thread_create_t));
    thread_count++;
}

unsigned static_init_thread_count()
{
    return thread_count;
}

unsigned static_init_process_count()
{
    return process_count;
}

const struct OS_thread_create_t * static_init_thread_table()
{
    return os_thread_definitions;
}

const struct OS_process_definition_t * static_init_process_table()
{
    return os_process_definitions;
}


