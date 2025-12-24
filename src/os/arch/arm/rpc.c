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

void rpc_return(void);

/* To verify that we are using the right type and compiler won't cull
 * our data.
 */
_Static_assert(sizeof(unsigned long) == sizeof(void *), "Unsigned long type size differs from architecture pointer size.");

int os_rpc_call(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;
	#if __FPU_USED
	Thread_t current_thread = os_get_current_thread();
	const bool fpu_used = os_is_thread_using_fpu(current_thread);
	#else
	const bool fpu_used = false;
	#endif
	ExceptionFrame * local_frame = (ExceptionFrame *) __get_PSP();
	sanitize_psp((uint32_t *) local_frame);
	RPC_Service_t * service = (RPC_Service_t *) get_exception_argument(local_frame, 4, fpu_used);
	VTable_t vtable = service->vtable;

	Process_t process_id = get_vtable_process(vtable);
	if (process_id == E_VTABLE_UNKNOWN)
	{
		return E_INVALID_ADDRESS;
	}
	
	if (!rpc_stack_push(process_id))
	{
		return E_IN_TOO_DEEP;
	}

	mpu_load((const MPU_State *) &os_processes[process_id].mpu, 0, MPU_HOSTED_STATE_SIZE);

	unsigned method_id = get_exception_argument(local_frame, 5, fpu_used);
	RPC_Method_t method = vtable[method_id];

#ifdef CMRX_RPC_CANARY
	unsigned canary = get_exception_argument(local_frame, 6, fpu_used);
	ASSERT(canary == 0xAA55AA55);
#endif

	ExceptionFrame * remote_frame = push_exception_frame(local_frame, 2, fpu_used);
	sanitize_psp((uint32_t *) remote_frame);

	// remote frame arg [1 .. 4] = local frame arg [0 .. 3]
	for (int q = 0; q < 4; ++q)
	{
		set_exception_argument(remote_frame, q + 1,
				get_exception_argument(local_frame, q, fpu_used),
				fpu_used
				);
	}

	set_exception_argument(remote_frame, 0, (uint32_t) service, fpu_used);

#ifdef CMRX_RPC_CANARY
	methods->set_exception_argument(remote_frame, 5, 0xAA55AA55, fpu_used);
#endif
	set_exception_pc_lr(remote_frame, method, rpc_return);
	
	__set_PSP((uint32_t) remote_frame);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it.
#if __FPU_USED
	// Store LR into r0 image inside exception frame on stack
	// It will be rewritten by the return value from RPC anyway
	return os_threads[current_thread].arch.exc_return;
#else
	// we will let rewrite R0 position in exception
	// stack frame by arg0 value, which actually is the same value
	return arg0;
#endif
}

int _rpc_call(void);

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;
#if __FPU_USED
	Thread_t current_thread = os_get_current_thread();
	bool fpu_used = os_is_thread_using_fpu(current_thread);
#else
	const bool fpu_used = false;
#endif

	ExceptionFrame * remote_frame = (ExceptionFrame *) __get_PSP();
	sanitize_psp((uint32_t *) remote_frame);
#ifdef CMRX_RPC_CANARY
	uint32_t canary = get_exception_argument(remote_frame, 5, fpu_used);

	ASSERT(canary == 0xAA55AA55);
#endif

	ExceptionFrame * local_frame = pop_exception_frame(remote_frame, 2, fpu_used);

#if __FPU_USED
	// Restore LR value saved into exception frame r0 slot
	// before the value in r0 slot will be overwritten by the return value
	// from the RPC call
	const uint32_t saved_exc_return = get_exception_argument(local_frame, 0, fpu_used);
	os_threads[current_thread].arch.exc_return = saved_exc_return;
	ASSERT(cortex_is_thread_psp_used(saved_exc_return));
	bool orig_fpu_used = os_is_thread_using_fpu(current_thread);
	if (fpu_used & !orig_fpu_used)
	{
		FPU->FPCCR &= ~(FPU_FPCCR_LSPACT_Msk);
	}
	fpu_used = orig_fpu_used;
#endif

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

	mpu_load((const MPU_State *) &os_processes[process_id].mpu, 0, MPU_HOSTED_STATE_SIZE);

	// Additional sanitizing
#if 0
	typeof(&_rpc_call) p_rpc_call = _rpc_call;
	p_rpc_call++;
	ASSERT(local_frame->pc == p_rpc_call);
	sanitize_psp((uint32_t *) local_frame);
#endif

	__set_PSP((uint32_t) local_frame);

	// we have manipulated PSP, but sv_call_handler doesn't know
	// about it. returning arg0 we will let it to write somewhere below
	// stack top, which *should* be harmless. The issue here is, that we
	// won't get return value written by sv_call_handler, so we have to
	// do it on our own.
	
	set_exception_argument(local_frame, 0, arg0, fpu_used);
	__ISB();
	
	return arg0;
}

/** @} */
