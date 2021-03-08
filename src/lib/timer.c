#include <cmrx/ipc/timer.h>
#include <cmrx/os/syscalls.h>

__SYSCALL int os_usleep(unsigned microseconds)
{
	__SVC(SYSCALL_USLEEP);
}

__SYSCALL int os_setitimer(unsigned microseconds)
{
	__SVC(SYSCALL_SETITIMER);
}
