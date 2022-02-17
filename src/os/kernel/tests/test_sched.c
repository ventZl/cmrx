//#include "../sched.c"
#include <stdbool.h>
#include <cmrx/os/mpu.h>
#include <cmrx/os/runtime.h>
#include <conf/kernel.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/sched/stack.h>
#include <string.h>

#include <conf/kernel.h>

#undef NULL
#include <ctest.h>

uint32_t sched_tick_increment = 1;
extern struct OS_stack_t os_stacks;

#define _ptr(x)	((void *) (x))
extern void provide_process_table(struct OS_process_definition_t * table, unsigned count);
extern void provide_thread_table(struct OS_thread_create_t * table, unsigned count);


bool schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
	return false;
}

int mpu_configure_region(uint8_t region, const void * base, uint32_t size, uint8_t flags, uint32_t * RBAR, uint32_t * RASR)
{
	return 0;
}

int __SVC(int svc)
{
	return 0;
}

bool mpu_enable_called = 0;

void mpu_enable()
{
	mpu_enable_called = 1;
}

int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state)
{
	return 0;
}

int thread_exit(int status)
{
	return 0;
}

int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint8_t flags)
{
	return 0;
}

bool systick_enable_called = false;

void systick_enable()
{
	systick_enable_called = true;
}

CTEST_DATA(init) {
};

CTEST_SETUP(init)
{
	mpu_enable_called = 0;
	systick_enable_called = 0;
	memset(os_threads, 0, sizeof(os_threads));
	memset(os_processes, 0, sizeof(os_processes));
	memset(&os_stacks, 0, sizeof(os_stacks));
}

CTEST2(init, process) 
{
	struct OS_process_definition_t ptable[] = {
		{
			{
				{ _ptr(0x1000), _ptr(0x2000) },
				{ _ptr(0x2000), _ptr(0x3000) },
				{ _ptr(0x3000), _ptr(0x4000) },
				{ _ptr(0x4000), _ptr(0x5000) }
			}, 
			{ 0, 0 }
		}
	};

	provide_process_table(ptable, 1);

	os_start();
	ASSERT_EQUAL(1, systick_enable_called);
	ASSERT_EQUAL((long) os_processes[0].definition, (long) &ptable[0]);
	ASSERT_EQUAL(0, os_threads[0].process_id);
	ASSERT_EQUAL(0, os_threads[1].priority);
	ASSERT_EQUAL(THREAD_STATE_RUNNING, os_threads[0].state);
	ASSERT_EQUAL(0x1, os_stacks.allocations);
	ASSERT_EQUAL(0, os_threads[0].stack_id);
}

int dummy_thread_entry(void * data)
{
	return 0;
}

CTEST2(init, thread)
{
	struct OS_process_definition_t ptable[] = {
		{
			{
				{ _ptr(0x1000), _ptr(0x2000) },
				{ _ptr(0x2000), _ptr(0x3000) },
				{ _ptr(0x3000), _ptr(0x4000) },
				{ _ptr(0x4000), _ptr(0x5000) }
			}, 
			{ 0, 0 }
		}
	};

	struct OS_thread_create_t ttable[] = {
		{
			&ptable[0],
			dummy_thread_entry,
			NULL,
			32
		}
	};

	provide_thread_table(ttable, 1);

	os_start();

	ASSERT_EQUAL(1, systick_enable_called);
	ASSERT_EQUAL((long) os_processes[0].definition, (long) &ptable[0]);

	ASSERT_EQUAL(0, os_threads[0].process_id);
	ASSERT_EQUAL(32, os_threads[0].priority);
	ASSERT_EQUAL(0, os_threads[0].stack_id);
	ASSERT_EQUAL(THREAD_STATE_RUNNING, os_threads[0].state);

	// TODO: Why is os_idle_thread bound to process 0?
	ASSERT_EQUAL(0, os_threads[1].process_id);
	ASSERT_EQUAL(1, os_threads[1].stack_id);
	ASSERT_EQUAL(255, os_threads[1].priority);
	ASSERT_EQUAL(THREAD_STATE_READY, os_threads[1].state);

	ASSERT_EQUAL(0x3, os_stacks.allocations);
}

CTEST(sched, yield) {

}

CTEST_DATA(stack) {
};

CTEST_SETUP(stack) 
{
	memset(&os_stacks, 0, sizeof(os_stacks));
}

extern int os_stack_create();
extern void os_stack_dispose(uint32_t stack_id);

CTEST2(stack, alloc)
{
	int stack;
	for (int q = 0; q < OS_STACKS; ++q)
	{
		stack = os_stack_create();
		ASSERT_NOT_EQUAL(-1, stack);
	}

	stack = os_stack_create();
	ASSERT_EQUAL(-1, stack);
}

CTEST2(stack, dispose)
{
	int stack;
	for (int q = 0; q < (2 * OS_STACKS); ++q)
	{
		stack = os_stack_create();
		ASSERT_NOT_EQUAL(-1, stack);
		os_stack_dispose(stack);
		ASSERT_EQUAL(0, os_stacks.allocations);
	}
}

CTEST2(stack, dispose_invalid)
{
	// Fake invalid internal stack allocation state
	// This is to check that disposal of stack out of range
	// won't corrupt allocation bitmap
	os_stacks.allocations = ~0;
	for (int q = OS_STACKS; q < 32; ++q)
	{
		os_stack_dispose(q);
		ASSERT_EQUAL(~0U, os_stacks.allocations);
	}
}


