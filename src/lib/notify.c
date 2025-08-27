#include <cmrx/ipc/signal.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL int notify_object(const void * object)
{
	(void) object;
	__SVC(SYSCALL_NOTIFY_OBJECT);
}

__SYSCALL int notify_object_immediate(const void * object)
{
	(void) object;
	__SVC(SYSCALL_NOTIFY_OBJECT_IMMEDIATE);
}

__SYSCALL int wait_for_object(const void * object, uint32_t timeout)
{
	(void) object;
	(void) timeout;
	__SVC(SYSCALL_WAIT_FOR_OBJECT);
}

__SYSCALL int wait_for_object_value(const uint8_t * object, uint8_t value, uint32_t timeout, uint32_t flags)
{
	(void) object;
	(void) timeout;
	(void) value;
	(void) flags;
	__SVC(SYSCALL_WAIT_FOR_OBJECT_VALUE);
}
