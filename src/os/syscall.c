#include <stdint.h>
#include <cmrx/sched.h>
#include <cmrx/intrinsics.h>
#include <cmrx/os/syscall.h>
#include <cmrx/os/sysenter.h>

#define SYSCALL_GETTID					0x0
#define SYSCALL_SCHED_YIELD				0x1
#define SYSCALL_THREAD_START			0x2

__SYSCALL int sched_yield()
{
	__SVC(SYSCALL_SCHED_YIELD);
}

__SYSCALL int get_tid()
{
	__SVC(SYSCALL_GETTID);
}

extern uint8_t os_get_current_thread(void);
extern int os_sched_yield(void);

struct Syscall_Entry_t syscalls[] = {
	{ SYSCALL_GETTID, (Syscall_Handler_t) &os_get_current_thread },
	{ SYSCALL_SCHED_YIELD, (Syscall_Handler_t) &os_sched_yield }
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


