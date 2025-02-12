#include <cmrx/ipc/signal.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL int notify_object(const void * object)
{
	(void) object;
	__SVC(SYSCALL_NOTIFY_OBJECT);
}

__SYSCALL int wait_for_object(const void * object, uint32_t timeout)
{
	(void) object;
	(void) timeout;
	__SVC(SYSCALL_WAIT_FOR_OBJECT);
}
