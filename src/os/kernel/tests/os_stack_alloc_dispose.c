//#include "../sched.c"
#include <stdbool.h>
#include <string.h>
#include <conf/kernel.h>
#undef NULL
#include <cmrx/os/runtime.h>
#include <ctest.h>

extern struct OS_stack_t os_stacks;

CTEST_DATA(stack) {
};

CTEST_SETUP(stack) 
{
    (void) data;
	memset(&os_stacks, 0, sizeof(os_stacks));
}

extern int os_stack_create();
extern void os_stack_dispose(uint32_t stack_id);

/* Test stack allocation mechanism.
 * Test that it is possible to allocate exactly
 * that many stacks, how many stacks were requested
 * in kernel configuration.
 */
CTEST2(stack, alloc)
{
    (void) data;
	int stack;
	for (int q = 0; q < OS_STACKS; ++q)
	{
		stack = os_stack_create();
		ASSERT_NOT_EQUAL(-1, stack);
	}

	stack = os_stack_create();
	ASSERT_EQUAL(-1, stack);
}

/* Test that stack deallocation works.
 * Test if stack deallocation will keep the stack
 * allocator in clean and expected state.
 */
CTEST2(stack, dispose)
{
    (void) data;
	int stack;
	for (int q = 0; q < (2 * OS_STACKS); ++q)
	{
		stack = os_stack_create();
		ASSERT_NOT_EQUAL(-1, stack);
		os_stack_dispose(stack);
		ASSERT_EQUAL(0, os_stacks.allocations);
	}
}

/* Test attempt on out of bounds deallocation
 * Test if an attempt to deallocate out of bounds
 * stack will corrupt stack allocator state.
 */
CTEST2(stack, dispose_invalid)
{
    (void) data;
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


