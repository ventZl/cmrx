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

/* Save/restore s0-s11 (callee-saved) per psABI (riscv-abi documentation,
 * 1.1 Integer Register Convention section). Frame size keeps SP 16-byte aligned
 * (riscv-abi documentation, 2.2 Hardware Floating-point Calling Convention section,
 * "stack pointer shall be aligned to a 128-bit boundary upon procedure entry" and
 * "must remain aligned throughout procedure execution" sections).
 */
__attribute__((naked)) void os_riscv_context_switch_perform(void)
{
	__asm__ volatile(
		"addi sp, sp, -%[frame]\n\t"
		"sw s0, 0(sp)\n\t"
		"sw s1, 4(sp)\n\t"
		"sw s2, 8(sp)\n\t"
		"sw s3, 12(sp)\n\t"
		"sw s4, 16(sp)\n\t"
		"sw s5, 20(sp)\n\t"
		"sw s6, 24(sp)\n\t"
		"sw s7, 28(sp)\n\t"
		"sw s8, 32(sp)\n\t"
		"sw s9, 36(sp)\n\t"
		"sw s10, 40(sp)\n\t"
		"sw s11, 44(sp)\n\t"
		"la t0, cpu_context\n\t"
		"lw t1, %[old_task_off](t0)\n\t"
		"sw sp, %[thread_sp_off](t1)\n\t"
		"lw t2, %[new_task_off](t0)\n\t"
		"lw sp, %[thread_sp_off](t2)\n\t"
		"lw s0, 0(sp)\n\t"
		"lw s1, 4(sp)\n\t"
		"lw s2, 8(sp)\n\t"
		"lw s3, 12(sp)\n\t"
		"lw s4, 16(sp)\n\t"
		"lw s5, 20(sp)\n\t"
		"lw s6, 24(sp)\n\t"
		"lw s7, 28(sp)\n\t"
		"lw s8, 32(sp)\n\t"
		"lw s9, 36(sp)\n\t"
		"lw s10, 40(sp)\n\t"
		"lw s11, 44(sp)\n\t"
		"addi sp, sp, %[frame]\n\t"
		"ret\n\t"
		:
		: [frame] "i"(CMRX_RISCV_CONTEXT_FRAME_BYTES),
		  [old_task_off] "i"(offsetof(struct OS_scheduling_context_t, old_task)),
		  [new_task_off] "i"(offsetof(struct OS_scheduling_context_t, new_task)),
		  [thread_sp_off] "i"(offsetof(struct OS_thread_t, sp))
		: "t0", "t1", "t2", "memory");
}

__attribute__((naked)) void os_riscv_context_switch_safe_point(void)
{
	__asm__ volatile(
		"mv t0, ra\n\t"
		"call os_riscv_context_switch_prepare\n\t"
		"mv ra, t0\n\t"
		"beqz a0, 1f\n\t"
		"j os_riscv_context_switch_perform\n\t"
		"1:\n\t"
		"ret\n\t"
		:
		:
		: "t0", "a0", "memory");
}
