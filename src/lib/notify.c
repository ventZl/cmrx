/** @ingroup api_signal
 * @{
 */
#include <cmrx/ipc/signal.h>
#include <cmrx/os/syscalls.h>

__SYSCALL int notify_object(const void *)
{
	__SVC(SYSCALL_NOTIFY_OBJECT);
}

__SYSCALL int wait_for_object(const void *, uint32_t)
{
	__SVC(SYSCALL_WAIT_FOR_OBJECT);
}

/** @} */
