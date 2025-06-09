#include <kernel/runtime.h>
#include <stdbool.h>
#include <stdint.h>

Thread_t updated_syscall_return_thread = 0xFF;
uint32_t updated_syscall_return_value = 0;
bool updated_syscall_return_called = false;


void os_boot_thread(Thread_t boot_thread)
{
    (void) boot_thread;
}

void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
    (void) thread;
    (void) stack_size;
    (void) entrypoint;
    (void) data;
    return;
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

int os_set_syscall_return_value(Thread_t thread_id, int32_t retval)
{
	updated_syscall_return_called = true;
	updated_syscall_return_thread = thread_id;
	updated_syscall_return_value = retval;
	return E_OK;
}
