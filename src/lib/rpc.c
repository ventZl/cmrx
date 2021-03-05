#include <cmrx/ipc/rpc.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>

__SYSCALL static int __rpc_call(unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, void * service, unsigned method)
{
	__SVC(SYSCALL_RPC_CALL);
}

int _rpc_call(void * service, unsigned method, unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3)
{
	return __rpc_call(arg0, arg1, arg2, arg3, service, method);
}
