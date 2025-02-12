#include <cmrx/ipc/rpc.h>
#include <cmrx/sys/syscalls.h>
#include <stdarg.h>
#include <cmrx/assert.h>
#include <arch/sysenter.h>

__SYSCALL int _rpc_call(unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, void * service, unsigned method, unsigned canary)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;
    (void) service;
    (void) method;
    (void) canary;
	__SVC(SYSCALL_RPC_CALL);
}
