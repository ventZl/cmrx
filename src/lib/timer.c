#include <cmrx/ipc/timer.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL int usleep(unsigned microseconds)
{
    (void) microseconds;
	__SVC(SYSCALL_USLEEP, microseconds);
}

__SYSCALL int setitimer(unsigned microseconds)
{
    (void) microseconds;
	__SVC(SYSCALL_SETITIMER, microseconds);
}
