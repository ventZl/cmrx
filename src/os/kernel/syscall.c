#include <stdint.h>
#include <cmrx/sched.h>
#include <cmrx/intrinsics.h>
#include <cmrx/os/syscall.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>
#include <cmrx/os/rpc.h>

__SYSCALL int sched_yield()
{
	__SVC(SYSCALL_SCHED_YIELD);
}

__SYSCALL int get_tid()
{
	__SVC(SYSCALL_GET_TID);
}

#if 0
__SYSCALL int mutex_init(mutex_t * restrict mutex)
{
	__SVC(SYSCALL_MUTEX_INIT);
}

__SYSCALL int mutex_destroy(mutex_t * mutex)
{
	__SVC(SYSCALL_MUTEX_DESTROY);
}

__SYSCALL int mutex_lock(mutex_t * mutex)
{
	__SVC(SYSCALL_MUTEX_LOCK);
}

__SYSCALL int mutex_unlock(mutex_t * mutex)
{
	__SVC(SYSCALL_MUTEX_UNLOCK);
}

__SYSCALL int mutex_trylock(mutex_t * mutex)
{
	__SVC(SYSCALL_MUTEX_TRYLOCK);
}
#endif

extern uint8_t os_get_current_thread(void);
extern int os_sched_yield(void);

struct Syscall_Entry_t syscalls[] = {
	{ SYSCALL_GET_TID, (Syscall_Handler_t) &os_get_current_thread },
	{ SYSCALL_SCHED_YIELD, (Syscall_Handler_t) &os_sched_yield },
	{ SYSCALL_RPC_CALL, (Syscall_Handler_t) &os_rpc_call },
	{ SYSCALL_RPC_RETURN, (Syscall_Handler_t) &os_rpc_return }
};

/*__attribute__((naked))*/ void sv_call_handler(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	uint32_t * psp = (uint32_t *) __get_PSP();
	uint16_t * lra = (uint16_t *) *(psp + 6);
	uint8_t syscall_id = *(lra - 1);
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
	return;
}


