#include <cmrx/os/sched.h>

#undef NULL
#include <ctest.h>
#include <string.h>

#define _ptr(x)	((void *) (x))
extern void provide_process_table(struct OS_process_definition_t * table, unsigned count);
extern void provide_thread_table(struct OS_thread_create_t * table, unsigned count);

CTEST_DATA(init) {
};

CTEST_SETUP(init)
{
    (void) data;
	memset(os_threads, 0, sizeof(os_threads));
	memset(os_processes, 0, sizeof(os_processes));
	memset(&os_stacks, 0, sizeof(os_stacks));
}

/* Test process initialization.
 * This tests that process initialization done from static
 * process table is performed correctly.
 */
CTEST2(init, process) 
{
    (void) data;
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
	ASSERT_EQUAL((long) os_processes[0].definition, (long) &ptable[0]);
	ASSERT_EQUAL(0, os_threads[0].process_id);
	ASSERT_EQUAL(0, os_threads[1].priority);
	ASSERT_EQUAL(THREAD_STATE_RUNNING, os_threads[0].state);
	ASSERT_EQUAL(0x1, os_stacks.allocations);
	ASSERT_EQUAL(0, os_threads[0].stack_id);
}

int dummy_thread_entry(void * data)
{
    (void) data;
	return 0;
}

/* Test thread initialization.
 * This tests that thread initialization done from static
 * thread autostart table is performed correctly.
 */
CTEST2(init, thread)
{
    (void) data;
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


