/** @ingroup api_signal
 * @{
 */
#include <cmrx/ipc/signal.h>
#include <cmrx/os/syscalls.h>

__SYSCALL int signal(int signo, void (*sighandler)(uint32_t))
{
	__SVC(SYSCALL_SIGNAL);
}

__SYSCALL int kill(int thread, uint32_t signal)
{
	__SVC(SYSCALL_KILL);
}

/** @} */
