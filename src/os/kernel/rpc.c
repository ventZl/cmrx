#include <cmrx/os/rpc.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>
#include <cmrx/os/syscall.h>
#include <cmrx/intrinsics.h>

#include <cmrx/assert.h>
#include <cmrx/os/sanitize.h>

struct RPC_Service_t_;

typedef struct RPC_Service_t_ RPC_Service_t;

typedef int (*RPC_Method_t)(RPC_Service_t * service, unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3);

typedef RPC_Method_t * VTable_t ;

struct RPC_Service_t_ {
	VTable_t * vtable;
};

__SYSCALL void rpc_return()
{
	__SVC(SYSCALL_RPC_RETURN);
}

int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	ExceptionFrame * local_frame = (ExceptionFrame *) __get_PSP();
	sanitize_psp((uint32_t *) local_frame);
	RPC_Service_t * service = (void *) get_exception_argument(local_frame, 4);
	VTable_t * vtable = service->vtable;
	unsigned method_id = get_exception_argument(local_frame, 5); 
	RPC_Method_t * method = vtable[method_id];
	unsigned canary = get_exception_argument(local_frame, 6);

	ASSERT(canary == 0xAA55AA55);

	ExceptionFrame * remote_frame = push_exception_frame(local_frame, 2);
	sanitize_psp((uint32_t *) remote_frame);

	// remote frame arg [1 .. 4] = local frame arg [0 .. 3]
	for (int q = 0; q < 4; ++q)
	{
		set_exception_argument(remote_frame, q + 1,
				get_exception_argument(local_frame, q)
				);
	}

	set_exception_argument(remote_frame, 0, (uint32_t) service);
	set_exception_argument(remote_frame, 5, 0xAA55AA55);
	set_exception_pc_lr(remote_frame, method, rpc_return);
	
	__set_PSP((uint32_t *) remote_frame);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. we will let rewrite R0 position in exception
	// stack frame by arg0 value, which actually is the same value
	return arg0;
}

int _rpc_call();

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	ExceptionFrame * remote_frame = (ExceptionFrame *) __get_PSP();
	sanitize_psp((uint32_t *) remote_frame);
	uint32_t canary = get_exception_argument(remote_frame, 5);

	ASSERT(canary == 0xAA55AA55);

	ExceptionFrame * local_frame = pop_exception_frame(remote_frame, 2);
	
	typeof(&_rpc_call) p_rpc_call = _rpc_call;
	p_rpc_call++;
	ASSERT(local_frame->pc == p_rpc_call);
	canary = get_exception_argument(local_frame, 6);
	ASSERT(canary == 0xAA55AA55);

	sanitize_psp((uint32_t *) local_frame);
	__set_PSP((uint32_t *) local_frame);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. returning arg0 we will let it to write somewhere below
	// stack top, which *should* be harmless. The issue here is, that we
	// won't get return value written by sv_call_handler, so we have to
	// do it on our own.
	
	set_exception_argument(local_frame, 0, arg0);
	__ISB();
	
	return arg0;
}
