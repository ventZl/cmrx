#include "cmrx/defines.h"
#include <stdint.h>
#include <stdbool.h>

#include <cmrx/sys/syscalls.h>
#include "syscall.h"
#include <cmrx/assert.h>
#include <arch/sysenter.h>
#include <arch/syscall.h>

#include "rpc.h"
#include "sched.h"
#include "timer.h"
#include "signal.h"
#include "notify.h"
#include "txn.h"

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

REGISTER_SYSCALLS(
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
	{ SYSCALL_SETPRIORITY, (Syscall_Handler_t) &os_setpriority },
	{ SYSCALL_INIT_WAITABLE_OBJECT, (Syscall_Handler_t) &os_initialize_waitable_object },
	{ SYSCALL_NOTIFY_OBJECT, (Syscall_Handler_t) &os_sys_notify_object },
	{ SYSCALL_WAIT_FOR_OBJECT, (Syscall_Handler_t) &os_sys_wait_for_object },
	{ SYSCALL_CPUFREQ_GET, (Syscall_Handler_t) &os_cpu_freq_get },
	{ SYSCALL_GET_MICROTIME, (Syscall_Handler_t) &os_get_micro_time },
	{ SYSCALL_SHUTDOWN, (Syscall_Handler_t) &os_shutdown },
	{ SYSCALL_NOTIFY_OBJECT2, (Syscall_Handler_t) &os_sys_notify_object2 },
	{ SYSCALL_WAIT_FOR_OBJECT_VALUE, (Syscall_Handler_t) &os_sys_wait_for_object_value }
);

#pragma GCC diagnostic pop

static struct Syscall_Entry_t os_syscall_cache[_SYSCALL_COUNT];

int os_system_call_init(void)
{
	for (unsigned q = 0; q < _SYSCALL_COUNT; ++q)
	{
		os_syscall_cache[q].id = 0xFF;
	}
	for (struct Syscall_Entry_t * syscall = os_syscalls_start(); syscall < os_syscalls_end(); ++syscall)
	{
		os_syscall_cache[syscall->id] = *syscall;
/*		unsigned q;
		for (q = 0; q < syscalls_count; ++q)
		{
			if (os_syscall_cache[q].id >= syscall->id)
			{
				for (unsigned w = syscalls_count; w > q; --w)
				{
					os_syscall_cache[w] = os_syscall_cache[w-1];
				}

				break;
			}
		}

		os_syscall_cache[q] = *syscall;
		syscalls_count++;*/
	}

	return E_OK;
}

int os_system_call(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, uint8_t syscall_id)
{
/*
	unsigned lower = 0, upper = syscalls_count, current = upper / 2;

	while (os_syscall_cache[current].id != syscall_id) {
		if (os_syscall_cache[current].id < syscall_id) {
			lower = current;
			current = lower + (upper - lower) / 2;
		} else {
			if (os_syscall_cache[current].id > syscall_id) {
				upper = current;
				current = lower + (upper - lower) / 2;
			}
		}
	}*/

	if (syscall_id < _SYSCALL_COUNT)
	{
		struct Syscall_Entry_t * syscall = &os_syscall_cache[syscall_id];
		if (syscall->id == syscall_id) {
			uint32_t rv = syscall->handler(arg0, arg1, arg2, arg3);
			return rv; /*asm volatile("BX lr");*/
		}
	}

    return E_NOTAVAIL;
}
