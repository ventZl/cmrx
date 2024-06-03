/** @defgroup arch_arm_os OS services implementation
 * @ingroup arch_arm
 * @{
 */
#include <stdint.h>
#include <cmrx/os/runtime.h>
#include <arch/cortex.h>
#include <conf/kernel.h>

#include <stdbool.h>
#include <cmrx/assert.h>
#include <cmrx/os/sanitize.h>

#include <cmrx/os/context.h>
#include <arch/scb.h>

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include <cmrx/os/mpu.h>
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
    ASSERT(__get_LR() == 0xFFFFFFFDU);

	ctxt_switch_pending = false;
	sanitize_psp(cpu_context.old_task->sp);

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
