#include <cmrx/os/rpc.h>
#include <cmrx/os/sysenter.h>
#include <cmrx/os/syscalls.h>
#include <cmrx/intrinsics.h>

struct RPC_Service_t_;

typedef struct RPC_Service_t_ RPC_Service_t;

typedef int (*RPC_Method_t)(RPC_Service_t * service, unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3);

typedef RPC_Method_t * VTable_t ;

struct RPC_Service_t_ {
	VTable_t * vtable;
};

__SYSCALL static int rpc_return()
{
	__SVC(SYSCALL_RPC_RETURN);
}

int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	uint32_t * psp = (uint32_t *) __get_PSP();
	RPC_Service_t * service_id = (void *) *(psp + 8);
	VTable_t * vtable = service_id->vtable;
	unsigned method_id = (unsigned) *(psp + 9); 
	RPC_Method_t * method = vtable[method_id];

	psp -= 8;

	// Copy R0, R1, R2, R3, R12 from caller stack frame to callee stack frame
	for (int q = 0; q < 5; ++q)
	{
		*(psp + q) = *(psp + q + 8);
	}
	// LR
	*(psp + 5) = (uint32_t) &rpc_return;

	// PC
	*(psp + 6) = (uint32_t) method;

	// xPSR copy
	*(psp + 7) = *(psp + 8 + 7);

	__set_PSP(psp);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. we will let rewrite R0 position in exception
	// stack frame by arg0 value, which actually is the same value
	return arg0;
}

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	uint32_t * psp = (uint32_t *) __get_PSP();

	psp += 8;

	__set_PSP(psp);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. returning arg0 we will let it to write somewhere below
	// stack top, which *should* be harmless. The issue here is, that we
	// won't get return value written by sv_call_handler, so we have to
	// do it on our own.
	
	*(psp) = arg0;
	
	return arg0;
}
