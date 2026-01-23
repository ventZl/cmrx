#include <arch/corelocal.h>
#include <cmrx/arch/riscv/context_switch.h>
#include <ctest.h>
#include <kernel/context.h>
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <stdint.h>
#include <string.h>

#include "riscv_context_switch_fake.h"

static uint32_t riscv_stack_a[64] __attribute__((aligned(16)));
static uint32_t riscv_stack_b[64] __attribute__((aligned(16)));

static intmax_t riscv_ptr_value(const void *ptr)
{
	return (intmax_t)(uintptr_t) ptr;
}

static void riscv_reset_state(void)
{
	memset(&cpu_context, 0, sizeof(cpu_context));
	memset(os_threads, 0, sizeof(os_threads));
	memset(core, 0, sizeof(core));
	os_riscv_context_switch_request(false);
	riscv_context_switch_called = false;
	riscv_context_switch_perform_switch = true;
	riscv_context_switch_fake_sp = NULL;
}

static void riscv_setup_switch(uint32_t **old_sp, uint32_t **new_saved_sp, uint32_t **new_sp_after)
{
	riscv_reset_state();

	core[0].thread_current = 0;
	core[0].thread_next = 1;

	os_threads[0].state = THREAD_STATE_RUNNING;
	os_threads[1].state = THREAD_STATE_READY;

	cpu_context.old_task = &os_threads[0];
	cpu_context.new_task = &os_threads[1];

	*old_sp = &riscv_stack_a[64];
	*new_sp_after = &riscv_stack_b[64];
	*new_saved_sp = *new_sp_after - CMRX_RISCV_CONTEXT_FRAME_WORDS;

	cpu_context.new_task->sp = *new_saved_sp;
	riscv_context_switch_fake_sp = *old_sp;
}

CTEST_DATA(riscv_context_switch_reset) {
};

CTEST_SETUP(riscv_context_switch_reset) {
	riscv_reset_state();
}

CTEST2(riscv_context_switch_reset, request_set_clear)
{
	os_riscv_context_switch_request(false);
	ASSERT_EQUAL(os_riscv_context_switch_is_pending(), false);

	os_riscv_context_switch_request(true);
	ASSERT_EQUAL(os_riscv_context_switch_is_pending(), true);

	os_riscv_context_switch_request(false);
	ASSERT_EQUAL(os_riscv_context_switch_is_pending(), false);
}

CTEST_DATA(riscv_context_switch_safe_point) {
	uint32_t *old_sp;
	uint32_t *new_saved_sp;
	uint32_t *new_sp_after;
};

CTEST_SETUP(riscv_context_switch_safe_point) {
	uint32_t *old_sp = NULL;
	uint32_t *new_saved_sp = NULL;
	uint32_t *new_sp_after = NULL;

	memset(data, 0, sizeof(*data));
	riscv_setup_switch(&old_sp, &new_saved_sp, &new_sp_after);
	data->old_sp = old_sp;
	data->new_saved_sp = new_saved_sp;
	data->new_sp_after = new_sp_after;
}

CTEST2(riscv_context_switch_safe_point, safe_point_consumes_once)
{
	os_riscv_context_switch_request(true);

	os_riscv_context_switch_safe_point();
	ASSERT_EQUAL(os_riscv_context_switch_is_pending(), false);

	uint32_t *saved_old_sp = cpu_context.old_task->sp;
	os_riscv_context_switch_safe_point();
	ASSERT_EQUAL(riscv_ptr_value(cpu_context.old_task->sp), riscv_ptr_value(saved_old_sp));
}

CTEST2(riscv_context_switch_safe_point, sp_bookkeeping_and_state_transition)
{
	os_riscv_context_switch_request(true);
	os_riscv_context_switch_safe_point();

	ASSERT_EQUAL(riscv_ptr_value(cpu_context.old_task->sp),
				 riscv_ptr_value(data->old_sp - CMRX_RISCV_CONTEXT_FRAME_WORDS));
	ASSERT_EQUAL(riscv_ptr_value(riscv_context_switch_fake_sp),
				 riscv_ptr_value(data->new_sp_after));

	ASSERT_EQUAL(core[0].thread_current, 1);
	ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_READY);
	ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_RUNNING);
}
