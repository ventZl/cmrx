#include <stdint.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched.h>
#include <libopencm3/cm3/scb.h>
#include <cmrx/intrinsics.h>

#include <conf/kernel.h>

#include <stdbool.h>
#include <cmrx/assert.h>
#include <cmrx/os/sanitize.h>
#include <cmrx/os/sched/stack.h>

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include "mpu.h"
#endif

static struct OS_thread_t * old_task;
static struct OS_thread_t * new_task;
static uint8_t new_thread_id;
static bool ctxt_switch_pending;

extern struct OS_stack_t os_stacks;

/** Opt for task switch.
 * Calling this function will prepare task switch. It will set up
 * some stuff needed and then schedule PendSV.
 * @param current_task ID of task, which is currently running
 * @param next_task ID of task, which should be scheduled next
 * @note API of this function is dumb.
 */
bool schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
	if (ctxt_switch_pending)
	{
		// can this be any more robust?
		return false;
	}

	old_task = &os_threads[current_task];
	if (os_threads[current_task].state == THREAD_STATE_RUNNING)
	{
		// only mark leaving thread as ready, if it was runnig before
		// if leaving thread was, for example, quit before calling
		// os_sched_yield, then this would return it back to life
		os_threads[current_task].state = THREAD_STATE_READY;
	}
	new_task = &os_threads[next_task];
	new_thread_id = next_task;
	os_threads[next_task].state = THREAD_STATE_RUNNING;

	SCB_ICSR |= SCB_ICSR_PENDSVSET;

	__ISB();
	__DSB();

	return true;
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
	ASSERT(__get_LR() == (void *) 0xFFFFFFFD);

	old_task->sp = save_context();
	sanitize_psp(old_task->sp);

#ifdef KERNEL_HAS_MEMORY_PROTECTION
	mpu_store(&old_task->mpu);
	mpu_restore(&new_task->mpu);
#endif
	
	mpu_set_region(4, &os_stacks.stacks[new_thread_id], sizeof(os_stacks.stacks[new_thread_id]), MPU_RW);
	load_context(new_task->sp);
	sanitize_psp(new_task->sp);

	ctxt_switch_pending = false;

	asm volatile(
			"pop {pc}"
	);
}

