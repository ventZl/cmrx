#include <stdint.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/cortex.h>
#include <cmrx/intrinsics.h>

#include <conf/kernel.h>

#include <stdbool.h>
#include <cmrx/assert.h>
#include <cmrx/os/sanitize.h>
#include <cmrx/os/sched/stack.h>
#include <cmrx/os/signal.h>

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include <cmrx/os/mpu.h>
#endif

static struct OS_thread_t * old_task;
static struct OS_thread_t * new_task;

static struct OS_process_t * old_parent_process;
static struct OS_process_t * new_parent_process;

static struct OS_process_t * old_host_process;
static struct OS_process_t * new_host_process;

static uint8_t new_thread_id;
static Process_t new_process_id;
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

	ctxt_switch_pending = true;

	old_task = &os_threads[current_task];
	old_parent_process = &os_processes[old_task->process_id];

	if (old_task->rpc_stack[0] != 0)
	{
		old_host_process = &os_processes[old_task->rpc_stack[old_task->rpc_stack[0]]];
	}
	else
	{
		old_host_process = old_parent_process;
	}

	if (os_threads[current_task].state == THREAD_STATE_RUNNING)
	{
		// only mark leaving thread as ready, if it was runnig before
		// if leaving thread was, for example, quit before calling
		// os_sched_yield, then this would return it back to life
		os_threads[current_task].state = THREAD_STATE_READY;
	}
	new_task = &os_threads[next_task];
	new_parent_process = &os_processes[new_task->process_id];
	if (new_task->rpc_stack[0] != 0)
	{
		new_host_process = &os_processes[new_task->rpc_stack[new_task->rpc_stack[0]]];
	}
	else
	{
		new_host_process = new_parent_process;
	}

	new_thread_id = next_task;
	new_process_id = new_task->process_id;

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
	/* Do NOT put anything here. You will clobber context being stored! */
	asm volatile(
			".syntax unified\n\t"
			"push {lr}\n\t"
	);
	cm_disable_interrupts();
	/* Do NOT put anything here. You will clobber context being stored! */
	old_task->sp = save_context();
	ctxt_switch_pending = false;
	sanitize_psp(old_task->sp);

#ifdef KERNEL_HAS_MEMORY_PROTECTION
	if (old_parent_process != new_parent_process || old_host_process != new_host_process)
	{
//		mpu_store(&old_host_process->mpu, &old_parent_process->mpu);
		mpu_restore(&new_host_process->mpu, &new_parent_process->mpu);
	}
#endif
	
	// Configure stack for incoming process
	mpu_set_region(OS_MPU_REGION_STACK, &os_stacks.stacks[new_thread_id], sizeof(os_stacks.stacks[new_thread_id]), MPU_RW);
	sanitize_psp(new_task->sp);

	if (new_task->signals != 0 && new_task->signal_handler != NULL)
	{
		os_deliver_signal(new_task, new_task->signals);
		new_task->signals = 0;
	}
	load_context(new_task->sp);
	/* Do NOT put anything here. You will clobber context just restored! */
	cm_enable_interrupts();
	asm volatile(
			"pop {pc}"
	);
}

