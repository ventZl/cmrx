/** @ingroup api_signal
 * @{
 */
#include <cmrx/ipc/signal.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL int signal(int signo, void (*sighandler)(uint32_t))
{
    (void) signo;
    (void) sighandler;
	__SVC(SYSCALL_SIGNAL);
}

__SYSCALL int kill(int thread, uint32_t signal)
{
    (void) thread;
    (void) signal;
	__SVC(SYSCALL_KILL);
}

/** @} */
