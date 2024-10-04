/** @defgroup arch_arm_os OS services implementation
 * @ingroup arch_arm
 * @{
 */
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

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include <kernel/mpu.h>
#   include <arch/mpu_priv.h>
#endif

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
		mpu_restore(&cpu_context.new_host_process->mpu, &cpu_context.new_parent_process->mpu);
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
