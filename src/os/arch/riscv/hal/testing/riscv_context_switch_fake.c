#include <arch/corelocal.h>
#include <cmrx/arch/riscv/context_switch.h>
#include <kernel/context.h>
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <stdbool.h>
#include <stdint.h>

#include "riscv_context_switch_fake.h"

bool riscv_context_switch_called = false;
bool riscv_context_switch_perform_switch = true;
uint32_t *riscv_context_switch_fake_sp = NULL;

static volatile bool pending_context_switch[OS_NUM_CORES];

void os_riscv_context_switch_request(bool activate)
{
	pending_context_switch[coreid()] = activate;
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

static void os_riscv_context_switch_perform(void)
{
	uint32_t *sp = riscv_context_switch_fake_sp;
	if (sp != NULL)
	{
		sp -= CMRX_RISCV_CONTEXT_FRAME_WORDS;
	}
	cpu_context.old_task->sp = sp;

	sp = cpu_context.new_task->sp;
	if (sp != NULL)
	{
		sp += CMRX_RISCV_CONTEXT_FRAME_WORDS;
	}
	riscv_context_switch_fake_sp = sp;
	riscv_context_switch_called = true;
}

void os_riscv_context_switch_safe_point(void)
{
	if (!os_riscv_context_switch_is_pending())
	{
		return;
	}

	os_riscv_context_switch_request(false);

	if (!riscv_context_switch_perform_switch)
	{
		return;
	}

	os_riscv_context_switch_update_state();
	os_riscv_context_switch_perform();
}
