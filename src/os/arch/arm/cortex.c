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

/** Get value of process LR
 * @return top of application stack
 */
ALWAYS_INLINE void * __get_SP(void)
{
	void * sp;
	asm volatile(
		".syntax unified\n\t"
		"MOV %0, SP\n\t"
		: "=r" (sp)
	);

	return sp;
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
__attribute__((interrupt, naked)) void PendSV_Handler(void)
{
	// Disable interrupts and save the remaining thread registers not saved by the CPU
	asm volatile("CPSID I\n\t");
	SAVE_CONTEXT();
	// Performs: __set_LR(os_perform_thread_switch((uint32_t) __get_LR()));
	asm volatile(
		"MOV r0, LR\n\t"
		"BL %0\n\t"
		"MOV LR, r0\n\t"
		:
		: "i" (os_perform_thread_switch)
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
	cpu_context.old_task->sp = (uint32_t *) __get_PSP();

    // This assert checks that we are not preempting some other interrupt
    // handler. If you assert here, then your interrupt handler priority
    // is messed up. You need to configure PendSV to be the handler with
    // absolutely the lowest priority.
    ASSERT(LR == EXC_RETURN_THREAD_PSP);

	sanitize_psp(cpu_context.old_task->sp);

	struct OS_core_state_t * cpu_state = &core[coreid()];

	if (cpu_context.old_parent_process != cpu_context.new_parent_process
        || cpu_context.old_host_process != cpu_context.new_host_process)
	{
		mpu_restore((const MPU_State *) &cpu_context.new_host_process->mpu, (const MPU_State *) &cpu_context.new_parent_process->mpu);
	}

	// Configure stack for incoming process
    // This assumes that all stacks are of same size
	mpu_set_region(OS_MPU_REGION_STACK, cpu_context.new_stack, sizeof(os_stacks.stacks[0]), MPU_RW);
	sanitize_psp_for_thread(cpu_context.new_task->sp, cpu_state->thread_next);

	if (cpu_context.new_task->signals != 0 && cpu_context.new_task->signal_handler != NULL)
	{
		os_deliver_signal(cpu_context.new_task, cpu_context.new_task->signals);
		cpu_context.new_task->signals = 0;
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

	__set_PSP((uint32_t) cpu_context.new_task->sp);
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
__attribute__((interrupt)) void SVC_Handler(void)
{
	// This assert checks that we are not preempting some other interrupt
	// handler. If you assert here, then your interrupt handler priority
	// is messed up. You need to configure PendSV to be the handler with
	// absolutely the lowest priority.
	ASSERT(__get_LR() == (void *) 0xFFFFFFFDU);

	uint32_t * psp = (uint32_t *) __get_PSP();
	sanitize_psp(psp);
	ExceptionFrame * frame = (ExceptionFrame *) psp;
	uint16_t * lra = (uint16_t *) frame->pc;
	uint8_t syscall_id = *(lra - 1);
    uint32_t rv = os_system_call(frame->r0123[0], frame->r0123[1], frame->r0123[2], frame->r0123[3], syscall_id);
    *(psp) = rv;
    return; /*asm volatile("BX lr");*/
}

void os_core_sleep()
{
	__WFI();
}
/** @} */
