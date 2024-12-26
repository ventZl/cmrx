#include "cmrx/defines.h"
#include <stdint.h>
#include <stdbool.h>

#include <cmrx/sys/syscalls.h>
#include "syscall.h"
#include <cmrx/assert.h>
#include <arch/sysenter.h>

#include "rpc.h"
#include "sched.h"
#include "timer.h"
#include "signal.h"
#include "notify.h"

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

extern struct Syscall_Entry_t __syscall_start;
extern struct Syscall_Entry_t __syscall_end;

static SYSCALL_DEFINITION struct Syscall_Entry_t syscalls[] = {
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
	{ SYSCALL_SHUTDOWN, (Syscall_Handler_t) &os_shutdown }

};

#pragma GCC diagnostic pop

int os_system_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint8_t syscall_id)
{
	for (struct Syscall_Entry_t * syscall = &__syscall_start; syscall < &__syscall_end; ++syscall)
	{
		if (syscall->id == syscall_id)
		{
			uint32_t rv = syscall->handler(arg0, arg1, arg2, arg3);
			return rv; /*asm volatile("BX lr");*/
		}
	}

    return E_NOTAVAIL;
}
