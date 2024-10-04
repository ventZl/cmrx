/** @addtogroup os_rpc
 * @{
 */
#include "syscall.h"
#include <cmrx/sys/syscalls.h>
#include "rpc.h"
#include <arch/sysenter.h>
#include "sched.h"

/// @cond IGNORE
/// This is documented in the Kernel API group
__SYSCALL void rpc_return()
{
	__SVC(SYSCALL_RPC_RETURN);
}
/// @endcond

Process_t get_vtable_process(VTable_t * vtable)
{
	for (int q = 0; q < OS_PROCESSES; ++q)
	{
		if (os_processes[q].definition != NULL)
		{
			if ((VTable_t *) os_processes[q].definition->rpc_interface.start <= vtable 
					&& vtable < (VTable_t *) os_processes[q].definition->rpc_interface.end)
			{
				return q;
			}
		}
	}

	return E_VTABLE_UNKNOWN;
}

bool rpc_stack_push(Process_t process_id)
{
	Thread_t thread_id = os_get_current_thread();
	uint8_t depth = os_threads[thread_id].rpc_stack[0];
	if (depth < 8)
	{
		os_threads[thread_id].rpc_stack[depth + 1] = process_id;
		os_threads[thread_id].rpc_stack[0]++;
		return true;
	}

	return false;
}

int rpc_stack_pop()
{
	Thread_t thread_id = os_get_current_thread();
	uint8_t depth = os_threads[thread_id].rpc_stack[0];
	if (depth > 0)
	{
		// depth = 0 holds depth information
		os_threads[thread_id].rpc_stack[0]--;
		return depth - 1;
	}

	return 0;
}

Process_t rpc_stack_top()
{
	Thread_t thread_id = os_get_current_thread();
	uint8_t depth = os_threads[thread_id].rpc_stack[0];
	if (depth > 0)
	{
		return os_threads[thread_id].rpc_stack[depth];
	}
	return E_VTABLE_UNKNOWN;
}


/** @} */
