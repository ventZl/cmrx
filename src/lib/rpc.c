#include <cmrx/ipc/rpc.h>
#include <cmrx/sys/syscalls.h>
#include <stdarg.h>
#include <cmrx/assert.h>
#include <arch/sysenter.h>

__SYSCALL int _rpc_call(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, void * service, unsigned method, unsigned canary)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;
    (void) service;
    (void) method;
    (void) canary;
	__SVC(SYSCALL_RPC_CALL, arg0, arg1, arg2, arg3, service, method);
}
