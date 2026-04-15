#include <cmrx/arch/riscv/context_switch.h>
#include <arch/corelocal.h>
#include <kernel/context.h>
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static volatile bool pending_context_switch[OS_NUM_CORES];

#define os_riscv_context_switch_barrier() __asm__ volatile("" ::: "memory")

void os_riscv_context_switch_request(bool activate)
{
	pending_context_switch[coreid()] = activate;
	os_riscv_context_switch_barrier();
}

bool os_riscv_context_switch_is_pending(void)
{
	return pending_context_switch[coreid()];
}

static void os_riscv_context_switch_update_state(void)
{
	struct OS_core_state_t * const cpu_state = &core[coreid()];

	if (os_threads[cpu_state->thread_current].state == THREAD_STATE_RUNNING)
	{
		os_threads[cpu_state->thread_current].state = THREAD_STATE_READY;
	}

	cpu_state->thread_current = cpu_state->thread_next;
	os_threads[cpu_state->thread_current].state = THREAD_STATE_RUNNING;
}

__attribute__((noinline, used)) static uint32_t os_riscv_context_switch_prepare(void)
{
	if (!os_riscv_context_switch_is_pending())
	{
		return 0u;
	}

	os_riscv_context_switch_request(false);
	os_riscv_context_switch_update_state();
	return 1u;
}

/* Pure SP swap between old and new task.
 *
 * The full register context (ExceptionFrame) is saved/restored by the
 * trap handler assembly that calls the safe-point, not by this function.
 */
__attribute__((naked)) void os_riscv_context_switch_perform(void)
{
	__asm__ volatile(
		"la t0, cpu_context\n\t"
		"lw t1, %[old_task_off](t0)\n\t"
		"sw sp, %[thread_sp_off](t1)\n\t"
		"lw t2, %[new_task_off](t0)\n\t"
		"lw sp, %[thread_sp_off](t2)\n\t"
		"ret\n\t"
		:
		: [old_task_off] "i"(offsetof(struct OS_scheduling_context_t, old_task)),
		  [new_task_off] "i"(offsetof(struct OS_scheduling_context_t, new_task)),
		  [thread_sp_off] "i"(offsetof(struct OS_thread_t, sp))
		: "t0", "t1", "t2", "memory");
}

__attribute__((naked)) void os_riscv_context_switch_safe_point(void)
{
	__asm__ volatile(
		"mv s0, ra\n\t"
		"call os_riscv_context_switch_prepare\n\t"
		"mv ra, s0\n\t"
		"beqz a0, 1f\n\t"
		"j os_riscv_context_switch_perform\n\t"
		"1:\n\t"
		"ret\n\t"
		:
		:
		: "t0", "a0", "memory");
}
