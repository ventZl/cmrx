#include <RTE_Components.h>
#include CMSIS_device_header

#include <cmrx/sys/syscalls.h>
#include <kernel/syscall.h>
#include <cmrx/defines.h>

#include <stdint.h>
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <arch/cortex.h>
#include <conf/kernel.h>

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

void os_request_context_switch()
{
	SCB_ICSR |= SCB_ICSR_PENDSVSET;

	__ISB();
	__DSB();
}

/** Handle task switch.
 * This function performs the heavy lifting of context switching
 * when CPU is switched from one task to another.
 * As of now, it stores outgoing task's application context onto stack
 * and restores incoming task's context from its stack.
 * It then sets PSP to point to incoming task's stack and resumes
 * normal operation.
 */
__attribute__((naked)) void PendSV_Handler(void)
{
	/* Do NOT put anything here. You will clobber context being stored! */
	asm volatile(
			".syntax unified\n\t"
			"push {lr}\n\t"
	);
	cortex_disable_interrupts();
	/* Do NOT put anything here. You will clobber context being stored! */
	cpu_context.old_task->sp = save_context();

    // This assert checks that we are not preempting some other interrupt
    // handler. If you assert here, then your interrupt handler priority
    // is messed up. You need to configure PendSV to be the handler with
    // absolutely the lowest priority.
    ASSERT(__get_LR() == (void *) 0xFFFFFFFDU);

	sanitize_psp(cpu_context.old_task->sp);

	struct OS_core_state_t * cpu_state = &core[coreid()];

#ifdef KERNEL_HAS_MEMORY_PROTECTION
	if (cpu_context.old_parent_process != cpu_context.new_parent_process
        || cpu_context.old_host_process != cpu_context.new_host_process)
	{
//		mpu_store(&old_host_process->mpu, &old_parent_process->mpu);
		mpu_restore((const MPU_State *) &cpu_context.new_host_process->mpu, (const MPU_State *) &cpu_context.new_parent_process->mpu);
	}
#endif

	// Configure stack for incoming process
    // This assumes that all stacks are of same size
	mpu_set_region(OS_MPU_REGION_STACK, cpu_context.new_stack, sizeof(os_stacks.stacks[0]), MPU_RW);
	sanitize_psp(cpu_context.new_task->sp);

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

	load_context(cpu_context.new_task->sp);
	/* Do NOT put anything here. You will clobber context just restored! */
	__ISB();
	__DSB();

	cortex_enable_interrupts();
	asm volatile(
			"pop {pc}"
	);
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
 * @param arg0 syscall argument
 * @param arg1 syscall argument
 * @param arg2 syscall argument
 * @param arg3 syscall argument
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

/** @} */
