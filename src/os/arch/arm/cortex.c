#include <RTE_Components.h>
#include CMSIS_device_header

#include <cmrx/sys/syscalls.h>
#include <kernel/syscall.h>
#include <cmrx/defines.h>

#include <stdint.h>
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <arch/cortex.h>
#include <kernel/arch/context.h>

#include <stdbool.h>
#include <cmrx/assert.h>
#include <kernel/sanitize.h>

#include <kernel/context.h>
#include <arch/scb.h>

#include <kernel/mpu.h>
#include <arch/mpu_priv.h>

#include <arch/sysenter.h>
#include <arch/nvic.h>

#include "signal.h"
#include <stddef.h>

int os_nvic_enable(uint32_t irq)
{
    NVIC_EnableIRQ(irq);
    return E_OK;
}

int os_nvic_disable(uint32_t irq)
{
    NVIC_DisableIRQ(irq);
    return E_OK;
}

/** @defgroup arch_arm_os OS services implementation
 * @ingroup arch_arm
 * @{
 */

uint32_t * get_exception_arg_addr(ExceptionFrame * frame, unsigned argno, bool fp_active)
{
	ExceptionFrame * frame_alias = frame;
	ExceptionFrameFP * fp_frame_alias = (ExceptionFrameFP *) frame;
	if (argno < 4)
	{
		return &(frame_alias->r0123[argno]);
	}
	else
	{
		uint32_t * base;
		if (fp_active)
		{
			base = &fp_frame_alias->__spacer;
		}
		else
		{
			base = &frame_alias->xpsr;
		}

		if ((((*base) >> 9) & 1) == 1)
		{
			base += 2;
		}
		else
		{
			base += 1;
		}

		return &(base[argno - 4]);
	}
}


inline void os_request_context_switch(bool activate)
{
	/* Various Cortex-M manuals state that some fields
	 * of ICSR are writable, but ARM's own manual states
	 * that only SET and CLR bits are writable and everything
	 * else has writes ignored.
	 * So no ORing or ANDing needed here.
	 */
	if (activate)
	{
		SCB_ICSR = SCB_ICSR_PENDSVSET;
	}
	else
	{
		SCB_ICSR = SCB_ICSR_PENDSVCLR;
	}

	__ISB();
	__DSB();
}

uint32_t os_perform_thread_switch(uint32_t LR);

/** Wrapper for for Pending service interrupt handler.
 * CMRX uses pending service to handle heavy lifting of thread switching.
 * At the time PendSV is scheduled, the decision on which thread to run next
 * has already been made. This code just performs the switch.
 *
 * This assembly stub makes sure that all thread registers are properly saved
 * and restored during thread switch, so the actual implementation may be written
 * as plain C function without involving surprises.
 */
__attribute__((interrupt, naked)) void PendSV_Handler(void) // NOLINT - FPU register use is disabled via compiler flag
{
	// Disable interrupts and save the remaining thread registers not saved by the CPU
	asm volatile("CPSID I\n\t");
	SAVE_CONTEXT();
	// Performs: __set_LR(os_perform_thread_switch((uint32_t) __get_LR()));
	asm volatile(
		"MOV r0, LR\n\t"
		"BL os_perform_thread_switch\n\t"
		"MOV LR, r0\n\t"
		:
		:
		: "r0"
	);
	// Load registers not loaded by the CPU, enable interrupts and return from exception
	LOAD_CONTEXT();
	asm volatile(
		"CPSIE I\n\t"
		"BX LR");
}

/** Handle task switch.
 * This function performs the heavy lifting of context switching
 * when CPU is switched from one task to another.
 * It saves the SP of suspended  thread, then modifies the PSP to match the saved
 * value of incoming thread and if needed it performs signal handler injection
 * and/or MPU register reconfiguration.
 * Saving and restoring of registers is performed by the wrapper to avoid
 * C function preamble interference.
 * @param [in] LR copy of the LR value of the PendSV_Handler
 * @returns value that has to be used as new LR before PendSV_Handler quits
 */
uint32_t os_perform_thread_switch(uint32_t LR)
{
#if __FPU_USED
	cpu_context.old_task->arch.exc_return = LR;
#endif
	cpu_context.old_task->sp = (uint32_t *) __get_PSP();

	struct OS_thread_t * old_task = cpu_context.old_task;
    // This assert checks that we are not preempting some other interrupt
    // handler. If you assert here, then your interrupt handler priority
    // is messed up. You need to configure PendSV to be the handler with
    // absolutely the lowest priority.
	ASSERT(cortex_is_thread_psp_used(LR));

	sanitize_psp(old_task->sp);

	struct OS_core_state_t * cpu_state = &core[coreid()];

	os_save_fpu_context(old_task);

	if (cpu_context.old_parent_process != cpu_context.new_parent_process
        || cpu_context.old_host_process != cpu_context.new_host_process)
	{
		mpu_restore((const MPU_State *) &cpu_context.new_host_process->mpu, (const MPU_State *) &cpu_context.new_parent_process->mpu);
	}

	struct OS_thread_t * new_task = cpu_context.new_task;

	// Configure stack for incoming process
    // This assumes that all stacks are of same size
	mpu_init_stack(cpu_state->thread_next);
	sanitize_psp_for_thread(new_task->sp, cpu_state->thread_next);

	os_load_fpu_context(new_task);

	if (new_task->signals != 0 && new_task->signal_handler != NULL)
	{
		os_deliver_signal(new_task, new_task->signals);
		new_task->signals = 0;
	}
	if (os_threads[cpu_state->thread_current].state == THREAD_STATE_RUNNING)
	{
		// only mark leaving thread as ready, if it was runnig before
		// if leaving thread was, for example, quit before calling
		// os_sched_yield, then this would return it back to life
		os_threads[cpu_state->thread_current].state = THREAD_STATE_READY;
	}
	cpu_state->thread_current = cpu_state->thread_next;

	os_threads[cpu_state->thread_current].state = THREAD_STATE_RUNNING;

	/* Clear any PendSV requests that might have been made by ISR handlers
	 * preempting PendSV handler before it disabled interrupts
	 */
	os_request_context_switch(false);

#if __FPU_USED
	LR = new_task->arch.exc_return;
#endif

	__set_PSP((uint32_t) new_task->sp);
	__ISB();
	__DSB();
	sanitize_psp((uint32_t *) __get_PSP());

	return LR;
}

/** @} */

/** @defgroup arch_arm_syscall System calls implementation
 * @ingroup arch_arm
 * @{
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

static SYSCALL_DEFINITION struct Syscall_Entry_t nvic_syscalls[] = {
	{ SYSCALL_ENABLE_IRQ, (Syscall_Handler_t) &os_nvic_enable },
	{ SYSCALL_DISABLE_IRQ, (Syscall_Handler_t) &os_nvic_disable },
};

#pragma GCC diagnostic pop

/** ARM-specific entrypoint for system call handlers.
 *
 * This routine is common entrypoint for all syscall routines. It decodes
 * the syscall ID requested by userspace application and calls generic
 * method to service the system call. It is callable by executing the SVC
 * instruction. Code of SVC_Handler will retrieve the requested SVC ID and
 * let generic machinery to execute specified system call.
 */
uint32_t os_dispatch_system_call(uint32_t LR)
{
#if __FPU_USED
	Thread_t current_thread = os_get_current_thread();
	os_threads[current_thread].arch.exc_return = LR;
#endif
	// This assert checks that we are not preempting some other interrupt
	// handler. If you assert here, then your interrupt handler priority
	// is messed up. You need to configure PendSV to be the handler with
	// absolutely the lowest priority.
	ASSERT(cortex_is_thread_psp_used(LR));

	uint32_t * psp = (uint32_t *) __get_PSP();
	sanitize_psp(psp);
	ExceptionFrame * frame = (ExceptionFrame *) psp;
	uint16_t * lra = (uint16_t *) frame->pc;
	uint8_t syscall_id = *(lra - 1);
    uint32_t rv = os_system_call(frame->r0123[0], frame->r0123[1], frame->r0123[2], frame->r0123[3], syscall_id);
    *(psp) = rv;
#if __FPU_USED
    return os_threads[current_thread].arch.exc_return; /*asm volatile("BX lr");*/
#else
	return LR;
#endif
}

__attribute__((naked)) void SVC_Handler(void)
{
	asm volatile(
		"MOV r0, lr\n\t"
		"BL os_dispatch_system_call\n\t"
		"BX r0\n\t"
		:
		:
		: "r0"
	);
}

void os_core_sleep(void)
{
	__WFI();
}

static unsigned os_exception_frame_size(bool fp_used)
{
	return fp_used ? EXCEPTION_FRAME_FP_ENTRIES : EXCEPTION_FRAME_ENTRIES;
}

ExceptionFrame * push_exception_frame(ExceptionFrame * frame, unsigned args, bool fp_active)
{

	ExceptionFrame * frame_alias = frame;
	ExceptionFrame * outframe = (ExceptionFrame *) (((uint32_t *) frame) - (os_exception_frame_size(fp_active) + args));
	bool padding = false;

	// Check if forged frame is 8-byte aligned, or not
	if ((((uint32_t) outframe) & 7) != 0)
	{
		// Frame needs padding
		outframe = (ExceptionFrame *) (((uint32_t *) outframe) - 1);
		padding = true;
	}

	outframe->xpsr = frame_alias->xpsr;

	if (padding)
	{
		// we have padded the stack frame, clear STKALIGN in order to let
		// CPU know that original SP was 4-byte aligned
		outframe->xpsr |= 1 << 9;
	}
	else
	{
		// we didn't pad the stack frame, set STKALIGN in order to let
		// CPU know that original SP was 8-byte aligned
		outframe->xpsr &= ~(1 << 9);
	}

	return outframe;
}

ExceptionFrame * shim_exception_frame(ExceptionFrame * frame, unsigned args, bool fp_active)
{
	uint32_t * frame_alias = (uint32_t *) frame;

	ExceptionFrame * outframe = push_exception_frame(frame, args, fp_active);
	uint32_t * outframe_alias = (uint32_t *) outframe;

	const unsigned xpsr_offset = (offsetof(ExceptionFrame, xpsr) / sizeof(uint32_t));

	for (unsigned int q = 0; q < os_exception_frame_size(fp_active); ++q)
	{
		if (q == xpsr_offset)
		{
			// XPSR has been dealt with by push_exception_frame()
			continue;
		}
		outframe_alias[q] = frame_alias[q];
	}

	return outframe;
}

ExceptionFrame * pop_exception_frame(ExceptionFrame * frame, unsigned args, bool fp_active)
{
	ExceptionFrame * frame_alias = frame;
	ExceptionFrame * outframe = (ExceptionFrame *) (((uint32_t *) frame) + (os_exception_frame_size(fp_active) + args));
	if (((frame_alias->xpsr >> 9) & 1) == 1)
	{
		outframe = (ExceptionFrame *) (((uint32_t *) outframe) + 1);
	}

	// rewrite xPSR from pop-ped frame, retain value of bit 9
	outframe->xpsr = (outframe->xpsr & (1 << 9)) | (frame_alias->xpsr & ~(1 << 9));

	return outframe;
}

/** @} */
