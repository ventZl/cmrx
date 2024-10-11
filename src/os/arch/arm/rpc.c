/** @defgroup arch_arm_rpc RPC implementation
 * @ingroup arch_arm
 * 
 * Implementation of RPC mechanism for ARM Cortex-M processors.
 * 
 * When running on Cortex-M, remote procedure call is performed by injecting
 * an artificial exception frame which causes code to jump to the called routine
 * instead of returning to where it came from. Returning will then jump into specially
 * crafted routine, that injects rpc_return system call. This restores the previous 
 * state of caller's stack while copying the return value.
 * 
 * @{
 */
#include <kernel/rpc.h>
#include <kernel/syscall.h>
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <kernel/sanitize.h>

#include <arch/cortex.h>
#include <arch/mpu.h>
#include <arch/mpu_priv.h>

#include <cmrx/sys/syscalls.h>
#include <conf/kernel.h>
#include <cmrx/assert.h>


void rpc_return();

int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;
	ExceptionFrame * local_frame = (ExceptionFrame *) __get_PSP();
	sanitize_psp((uint32_t *) local_frame);
	RPC_Service_t * service = (void *) get_exception_argument(local_frame, 4);
	VTable_t * vtable = service->vtable;

	Process_t process_id = get_vtable_process(vtable);
	if (process_id == E_VTABLE_UNKNOWN)
	{
		return E_INVALID_ADDRESS;
	}
	
	if (!rpc_stack_push(process_id))
	{
		return E_IN_TOO_DEEP;
	}

	mpu_load(&os_processes[process_id].mpu, 0, MPU_HOSTED_STATE_SIZE);

	unsigned method_id = get_exception_argument(local_frame, 5); 
	RPC_Method_t * method = vtable[method_id];
/*	unsigned canary = get_exception_argument(local_frame, 6);

	ASSERT(canary == 0xAA55AA55);*/

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
	
	__set_PSP((uint32_t) remote_frame);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. we will let rewrite R0 position in exception
	// stack frame by arg0 value, which actually is the same value
	return arg0;
}

int _rpc_call();

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;
	ExceptionFrame * remote_frame = (ExceptionFrame *) __get_PSP();
	sanitize_psp((uint32_t *) remote_frame);
/*	uint32_t canary = get_exception_argument(remote_frame, 5);

	ASSERT(canary == 0xAA55AA55);*/

	ExceptionFrame * local_frame = pop_exception_frame(remote_frame, 2);
	
	int pstack_depth = rpc_stack_pop();
	Process_t process_id;

	if (pstack_depth > 0)
	{
		process_id = rpc_stack_top();
	}
	else
	{
		/* Warning for future wanderers: as of now, this returns
		 * process_id of current thread, which stores parent process.
		 * If I ever decide to change semantics to return current process
		 * ID, this may fail miserably.
		 */
		process_id = os_get_current_process();
	}


	if (process_id == E_VTABLE_UNKNOWN)
	{
		// here the process should probably die in segfault
		ASSERT(0);
	}

	mpu_load(&os_processes[process_id].mpu, 0, MPU_HOSTED_STATE_SIZE);

	// Additional sanitizing
#if 0
	typeof(&_rpc_call) p_rpc_call = _rpc_call;
	p_rpc_call++;
	ASSERT(local_frame->pc == p_rpc_call);
	canary = get_exception_argument(local_frame, 6);
	ASSERT(canary == 0xAA55AA55);

	sanitize_psp((uint32_t *) local_frame);
#endif
	__set_PSP((uint32_t) local_frame);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. returning arg0 we will let it to write somewhere below
	// stack top, which *should* be harmless. The issue here is, that we
	// won't get return value written by sv_call_handler, so we have to
	// do it on our own.
	
	set_exception_argument(local_frame, 0, arg0);
	__ISB();
	
	return arg0;
}

/** @} */
