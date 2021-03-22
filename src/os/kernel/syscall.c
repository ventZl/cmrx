#include <stdint.h>
#include <cmrx/intrinsics.h>
#include <cmrx/os/syscall.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>
#include <cmrx/os/rpc.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/timer.h>
#include <cmrx/os/signal.h>

#include <cmrx/os/sched/stack.h>
#include <cmrx/assert.h>
#include <cmrx/os/sanitize.h>

struct Syscall_Entry_t syscalls[] = {
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
	{ SYSCALL_KILL, (Syscall_Handler_t) &os_kill }
};

extern struct OS_stack_t os_stacks;

__attribute__((used)) void sv_call_handler(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	ASSERT(__get_LR() == (void *) 0xFFFFFFFD);
	uint32_t * psp = (uint32_t *) __get_PSP();
	sanitize_psp(psp);
	uint16_t * lra = (uint16_t *) *(psp + 6);
	uint8_t syscall_id = *(lra - 1);
	ASSERT(syscall_id < 16);
	for (unsigned q = 0; q < (sizeof(syscalls) / sizeof(syscalls[0])); ++q)
	{
		if (syscalls[q].id == syscall_id)
		{
			uint32_t rv = syscalls[q].handler(arg0, arg1, arg2, arg3);
			*(psp) = rv;
			return; /*asm volatile("BX lr");*/
		}
	}

	*(psp) = 0xFF;
	__ISB();
	__DSB();
	return;
}


