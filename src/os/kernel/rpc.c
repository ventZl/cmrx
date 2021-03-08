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

typedef struct {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	void * lr;
	void * pc;
	uint32_t xpsr;
	uint32_t arg4;
	uint32_t arg5;
	uint32_t arg6;
	uint32_t arg7;
} ExFrame;

__SYSCALL static int rpc_return()
{
	__SVC(SYSCALL_RPC_RETURN);
}

/* For Cortex-M3, keep this a multiple o 8 */
#define RPC_STACK_FRAME_SIZE			10

int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	uint32_t * psp = (uint32_t *) __get_PSP();
	RPC_Service_t * service = (void *) *(psp + 8);
	VTable_t * vtable = service->vtable;
	unsigned method_id = (unsigned) *(psp + 9); 
	RPC_Method_t * method = vtable[method_id];


	ExFrame * local_frame = (ExFrame *) psp;
	
	psp -= RPC_STACK_FRAME_SIZE;

	ExFrame * remote_frame = (ExFrame *) psp;

	remote_frame->r0 = (uint32_t) service;
	remote_frame->r1 = local_frame->r0;
	remote_frame->r2 = local_frame->r1;
	remote_frame->r3 = local_frame->r2;
	remote_frame->r12 = local_frame->r12;
	remote_frame->lr = &rpc_return;
	remote_frame->pc = method;
	remote_frame->xpsr = local_frame->xpsr;
	remote_frame->arg4 = local_frame->r3;

	__set_PSP(psp);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. we will let rewrite R0 position in exception
	// stack frame by arg0 value, which actually is the same value
	return arg0;
}

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	uint32_t * psp = (uint32_t *) __get_PSP();

	*(psp + 7 + RPC_STACK_FRAME_SIZE) = *(psp + 7);

	psp += RPC_STACK_FRAME_SIZE;

	__set_PSP(psp);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. returning arg0 we will let it to write somewhere below
	// stack top, which *should* be harmless. The issue here is, that we
	// won't get return value written by sv_call_handler, so we have to
	// do it on our own.
	
	*(psp) = arg0;
	__ISB();
	
	return arg0;
}
