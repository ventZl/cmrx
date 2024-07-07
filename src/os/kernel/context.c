#include <stdbool.h>
#include <stdint.h>
#include <cmrx/os/arch/context.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/context.h>

struct OS_scheduling_context_t cpu_context;
/*static struct OS_thread_t * old_task;
static struct OS_thread_t * new_task;

static struct OS_process_t * old_parent_process;
static struct OS_process_t * new_parent_process;

static struct OS_process_t * old_host_process;
static struct OS_process_t * new_host_process;

static uint32_t * new_stack;*/
//bool ctxt_switch_pending = false;

extern struct OS_stack_t os_stacks;

/** Schedule context switch on next suitable moment.
 *
 * This function will tell scheduler, that we want to switch running tasks.
 * Switch itself will be performed on next suitable moment by asynchronous
 * routine. This mechanism is used to avoid context switch in the middle of
 * interrupt routine, or otherwise complicated situation.
 *
 * @param current_task ID of thread which is currently being executed
 * @param next_task ID of thread which should start being executed
 * @returns true if context switch will happen, false otherwise
 */
bool schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
// This is probably useless with transactions now
#if 0

    if (ctxt_switch_pending)
	{
		// can this be any more robust?
		return false;
	}

	ctxt_switch_pending = true;
#endif

	cpu_context.old_task = &os_threads[current_task];
	cpu_context.old_parent_process = &os_processes[cpu_context.old_task->process_id];

	if (cpu_context.old_task->rpc_stack[0] != 0)
	{
		cpu_context.old_host_process = &os_processes[cpu_context.old_task->rpc_stack[cpu_context.old_task->rpc_stack[0]]];
	}
	else
	{
		cpu_context.old_host_process = cpu_context.old_parent_process;
	}

	if (os_threads[current_task].state == THREAD_STATE_RUNNING)
	{
		// only mark leaving thread as ready, if it was runnig before
		// if leaving thread was, for example, quit before calling
		// os_sched_yield, then this would return it back to life
		os_threads[current_task].state = THREAD_STATE_READY;
	}
	cpu_context.new_task = &os_threads[next_task];
	cpu_context.new_parent_process = &os_processes[cpu_context.new_task->process_id];
	if (cpu_context.new_task->rpc_stack[0] != 0)
	{
		cpu_context.new_host_process = &os_processes[cpu_context.new_task->rpc_stack[cpu_context.new_task->rpc_stack[0]]];
	}
	else
	{
		cpu_context.new_host_process = cpu_context.new_parent_process;
	}

    cpu_context.new_stack = os_stacks.stacks[cpu_context.new_task->stack_id];

	os_threads[next_task].state = THREAD_STATE_RUNNING;

    os_request_context_switch();

	return true;
}


