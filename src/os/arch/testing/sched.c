#include <cmrx/os/runtime.h>

void os_boot_thread(Thread_t boot_thread)
{
    (void) boot_thread;
}

uint32_t * os_thread_populate_stack(int stack_id, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
    (void) stack_id;
    (void) stack_size;
    (void) entrypoint;
    (void) data;
    return NULL;
}

int os_process_create(Process_t process_id, const struct OS_process_definition_t * definition)
{
	if (process_id >= OS_PROCESSES)
	{
		return E_OUT_OF_RANGE;
	}
	
	if (os_processes[process_id].definition != NULL)
	{
		return E_INVALID;
	}

	os_processes[process_id].definition = definition;

	return E_OK;
}

