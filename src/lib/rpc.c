#include <cmrx/ipc/rpc.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>
#include <stdarg.h>

__SYSCALL int __rpc_call(unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, void * service, unsigned method, uint32_t canary)
{
	__SVC(SYSCALL_RPC_CALL);
}

int _rpc_call(void * service, unsigned method, ...)
{
	register unsigned arg0, arg1, arg2, arg3;
	va_list args;
	va_start(args, method);
	arg0 = va_arg(args, unsigned);
	arg1 = va_arg(args, unsigned);
	arg2 = va_arg(args, unsigned);
	arg3 = va_arg(args, unsigned);
	va_end(args);
	return __rpc_call(arg0, arg1, arg2, arg3, service, method, 0xAA55AA55);
}
