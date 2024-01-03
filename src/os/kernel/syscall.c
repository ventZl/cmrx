#include "cmrx/defines.h"
#include <stdint.h>

#include <cmrx/os/syscalls.h>
#include <cmrx/os/syscall.h>
#include <cmrx/assert.h>

#include <cmrx/os/rpc.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/timer.h>
#include <cmrx/os/signal.h>

/** @defgroup os_syscall System calls
 * @ingroup os
 *
 * Portable parts of system call machinery.
 *
 */

/** List of syscalls provided by the kernel.
 * This table lists syscall ID and syscall handler for each supported
 * syscall.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

static const struct Syscall_Entry_t syscalls[] = {
	{ SYSCALL_GET_TID, (Syscall_Handler_t) &os_get_current_thread },
	{ SYSCALL_SCHED_YIELD, (Syscall_Handler_t) &os_sched_yield },
	{ SYSCALL_RPC_CALL, (Syscall_Handler_t) &os_rpc_call },
	{ SYSCALL_RPC_RETURN, (Syscall_Handler_t) &os_rpc_return },
	{ SYSCALL_THREAD_CREATE, (Syscall_Handler_t) &os_thread_create },
	{ SYSCALL_THREAD_JOIN, (Syscall_Handler_t) &os_thread_join },
	{ SYSCALL_THREAD_EXIT, (Syscall_Handler_t) &os_thread_exit },
	{ SYSCALL_USLEEP, (Syscall_Handler_t) &os_usleep },
	{ SYSCALL_SETITIMER, (Syscall_Handler_t) &os_setitimer },
	{ SYSCALL_SIGNAL, (Syscall_Handler_t) &os_signal },
	{ SYSCALL_KILL, (Syscall_Handler_t) &os_kill },
	{ SYSCALL_SETPRIORITY, (Syscall_Handler_t) &os_setpriority }
};

#pragma GCC diagnostic pop

int os_system_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint8_t syscall_id)
{
	ASSERT(syscall_id < (sizeof(syscalls) / sizeof(syscalls[0])));
	for (unsigned q = 0; q < (sizeof(syscalls) / sizeof(syscalls[0])); ++q)
	{
		if (syscalls[q].id == syscall_id)
		{
			uint32_t rv = syscalls[q].handler(arg0, arg1, arg2, arg3);
			return rv; /*asm volatile("BX lr");*/
		}
	}

    return E_NOTAVAIL;
}
