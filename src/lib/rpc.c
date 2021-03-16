#include <cmrx/ipc/rpc.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>
#include <stdarg.h>
#include <cmrx/assert.h>

__SYSCALL int _rpc_call(unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, void * service, unsigned method, unsigned canary)
{
	__SVC(SYSCALL_RPC_CALL);
}

