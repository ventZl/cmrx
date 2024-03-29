/** @ingroup api_timer
 * @{
 */
#include <cmrx/ipc/timer.h>
#include <cmrx/os/syscalls.h>

__SYSCALL int usleep(unsigned microseconds)
{
    (void) microseconds;
	__SVC(SYSCALL_USLEEP);
}

__SYSCALL int setitimer(unsigned microseconds)
{
    (void) microseconds;
	__SVC(SYSCALL_SETITIMER);
}

/** @} */
