#include <stdint.h>
#include <cmrx/os.h>
#include <libopencm3/cm3/scb.h>
#include <cmrx/intrinsics.h>

#include <conf/kernel.h>

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include "mpu.h"
#endif

static struct OS_thread_t * old_task;
static struct OS_thread_t * new_task;

/** Opt for task switch.
 * Calling this function will prepare task switch. It will set up
 * some stuff needed and then schedule PendSV.
 * @param current_task ID of task, which is currently running
 * @param next_task ID of task, which should be scheduled next
 * @note API of this function is dumb.
 */
void schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
	old_task = &os_threads[current_task];
	os_threads[current_task].state = TASK_STATE_READY;
	new_task = &os_threads[next_task];
	os_threads[next_task].state = TASK_STATE_RUNNING;

	SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

/** Handle task switch.
 * This function performs the heavy lifting of context switching
 * when CPU is switched from one task to another.
 * As of now, it stores outgoing task's application context onto stack
 * and restores incoming task's context from its stack.
 * It then sets PSP to point to incoming task's stack and resumes
 * normal operation.
 */
__attribute__((naked)) void pend_sv_handler(void)
{
	asm volatile(
			".syntax unified\n\t"
			"push {lr}\n\t"
	);

	old_task->sp = save_context();

#ifdef KERNEL_HAS_MEMORY_PROTECTION
	mpu_store(&old_task->mpu);
	mpu_restore(&new_task->mpu);
#endif
	
	load_context(new_task->sp);

	asm volatile(
			"pop {pc}"
	);
}

