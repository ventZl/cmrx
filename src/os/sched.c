#include <cmrx/os.h>
#include <cmrx/sched.h>
#include <cmrx/intrinsics.h>
#include <cmrx/pendsv.h>

#include <libopencm3/cm3/systick.h>

struct OS_thread_t os_threads[OS_TASKS];

static uint8_t task_current;
static uint8_t task_next;
static struct OS_stack_t os_stacks;

void sys_tick_handler(void)
{
	uint8_t task = task_current;

	do {
		if (os_threads[task].state == TASK_STATE_READY)
		{
			task_next = task;
			schedule_context_switch(task_current, task_next);
			task_current = task_next;
			return;
		}

		task = (task + 1) % OS_TASKS;
	} while (task != task_current);
}

static void os_task_dispose(void)
{
	// this should be called when task returns
}

void os_start(uint32_t tid)
{
	if (os_threads[tid].state != TASK_STATE_READY)
	{
		task_current = tid;
		os_threads[tid].state = TASK_STATE_RUNNING;
		uint32_t * task_sp = os_threads[tid].sp;

		__set_PSP(task_sp);
		__set_CONTROL(0x03); 	// SPSEL = 1 | nPRIV = 1: use PSP and unpriveldged thread mode
		__ISB();

		os_threads[tid].task->entrypoint();
	}
}

int os_stack_create()
{
	uint32_t stack_mask = 1;
	for(int q = 0; q < OS_STACKS; ++q)
	{
		if ((os_stacks.allocations & stack_mask) == 0)
		{
			os_stacks.allocations |= stack_mask;
			return q;
		}
		stack_mask *= 2;
	}

	return 0xFFFFFFF;
}

/** Organize for init task.
 * This is a special version of os_task_start()
 * which does not prepare stack for jump-in from
 * PendSV handler. In this case the stack is 
 * completely empty and it is expected to jump in
 * here from os_start().
 */
int os_task_init(int tid)
{
	if (os_threads[tid].state == TASK_STATE_CREATED)
	{
		uint32_t stack_id = os_stack_create();
		if (stack_id != ~0)
		{
			os_threads[tid].stack_id = stack_id;
			os_threads[tid].sp = &os_stacks.stacks[stack_id][OS_STACK_DWORD];
			return E_OK;
		}
		return E_OUT_OF_STACKS;
	}
	return E_TASK_RUNNING;
}

int os_task_start(int tid)
{
	if (tid < OS_TASKS)
	{
		if (os_threads[tid].state == TASK_STATE_CREATED)
		{
			uint32_t stack_id = os_stack_create();
			if (stack_id != 0xFFFFFFFF)
			{
				os_threads[tid].stack_id = stack_id;
				os_threads[tid].sp = &os_stacks.stacks[stack_id][OS_STACK_DWORD - 16];

				os_stacks.stacks[stack_id][OS_STACK_DWORD - 3] = (uint32_t) os_task_dispose; // LR
				os_stacks.stacks[stack_id][OS_STACK_DWORD - 2] = (uint32_t) os_threads[tid].task->entrypoint; // PC
				os_stacks.stacks[stack_id][OS_STACK_DWORD - 1] = 0x01000000; // xPSR

				os_threads[tid].state = TASK_STATE_READY;
				return E_OK;
			}
			else
			{
				return E_OUT_OF_STACKS;
			}
		}
		return E_TASK_RUNNING;
	}
	return E_OUT_OF_RANGE;
}

/*
 * Set up timer to fire every x milliseconds
 * This is a unusual usage of systick, be very careful with the 24bit range
 * of the systick counter!  You can range from 1 to 2796ms with this.
 */
void systick_setup(int xms)
{
	/* div8 per ST, stays compatible with M3/M4 parts, well done ST */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	/* clear counter so it starts right away */
	STK_CVR = 0;

	systick_set_reload(4000000);//rcc_ahb_frequency / 8 / 1000 * xms);
	systick_counter_enable();
	systick_interrupt_enable();
}

int os_task_create(const struct OS_task_t * task)
{
	for (int q = 0; q < OS_TASKS; ++q)
	{
		if (os_threads[q].state == TASK_STATE_EMPTY)
		{
			os_threads[q].stack_id = OS_TASK_NO_STACK;
			os_threads[q].task = task;
			os_threads[q].sp = (uint32_t *) ~0;
			os_threads[q].state = TASK_STATE_CREATED;
			return q;
		}
	}
	return ~0;
}


