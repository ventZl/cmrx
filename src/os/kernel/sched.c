#include <cmrx/os.h>
#include <cmrx/sched.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched/stack.h>
#include <cmrx/os/sched.h>
#include <cmrx/intrinsics.h>
#include <cmrx/pendsv.h>
#include <string.h>
#include <conf/kernel.h>
#include <cmrx/ipc/thread.h>

#include <libopencm3/cm3/systick.h>

struct OS_thread_t os_threads[OS_THREADS];

static uint8_t thread_current;
static uint8_t thread_next;
static struct OS_stack_t os_stacks;

extern const struct OS_process_t __applications_start;
const struct OS_process_t * const processes = &__applications_start;

extern const struct OS_thread_create_t __thread_create_start;
extern const struct OS_thread_create_t __thread_create_end;

const struct OS_thread_create_t * const autostart_threads = &__thread_create_start;

static int __os_thread_create(const struct OS_process_t * process, entrypoint_t * entrypoint, void * data, uint8_t priority);
static int os_thread_alloc(const struct OS_process_t * process, uint8_t priority);

static uint32_t sched_tick_increment;

static uint32_t sched_microtime;

__attribute__((noreturn)) static int os_idle_thread(void * data)
{
	while (1);
}

int os_sched_yield(void)
{
	uint8_t thread = thread_current;
	uint16_t best_prio = 0x1FF;
	uint8_t next_thread;

	do {
		if (os_threads[thread].state == THREAD_STATE_READY)
		{
			/* Schedule different thread only if it has lower numerical value of priority
			 * than one we've already found. This should enforce prioritized round robin
			 * scheduling
			 */
			if (os_threads[thread].priority < best_prio)
			{
				next_thread = thread;
				best_prio = os_threads[thread].priority;
			}
		}

		thread = (thread + 1) % OS_THREADS;
	} while (thread != thread_current);

	/* We actually found something to run */
	if (best_prio < 0x100)
	{
		/* TODO: better naming... */
		thread_next = next_thread;
		schedule_context_switch(thread_current, thread_next);
		thread_current = thread_next;
		return 0;
	}
	return 0;
}

void sys_tick_handler(void)
{
	sched_microtime += sched_tick_increment;
	os_sched_yield();
}

uint8_t os_get_current_thread(void)
{
	return thread_current;
}

uint8_t os_get_current_process(void)
{
	return os_threads[thread_current].process - &__applications_start;
}

uint32_t os_get_micro_time(void)
{
	return sched_microtime;
}

static void os_thread_dispose(int arg0)
{
	thread_exit(arg0);
	while(1);
	// this should be called when thread returns
}

void os_start()
{
	unsigned threads = &__thread_create_end - &__thread_create_start;

	memset(&os_threads, 0, sizeof(os_threads));

	for (unsigned q = 0; q < threads; ++q)
	{
		__os_thread_create(autostart_threads[q].process, autostart_threads[q].entrypoint, autostart_threads[q].data, autostart_threads[q].priority);
	}

	__os_thread_create(NULL, os_idle_thread, NULL, 0xFF);

	// Find first thread which is runnable and simply start executing it
	for (unsigned q = 0; q < OS_THREADS; ++q)
	{
		if (os_threads[q].state == THREAD_STATE_READY)
		{
			thread_current = q;
			os_threads[q].state = THREAD_STATE_RUNNING;
			// Alter thread's SP to so that it's stack is completely 
			// empty. Other threads have stack prepared for "return from
			// interrupt handler scenario".
			os_threads[q].sp = &os_stacks.stacks[os_threads[q].stack_id][OS_STACK_DWORD];

			/* These two variables are marked as register, because
			 * call to __set_CONTROL() below will switch to 
			 * process SP being used. Yet if this is being built
			 * with -O0, entrypoint and data would end up on stack
			 * being referenced by SP. After SP changes, those
			 * references would be messed up. By forcing these to be
			 * register variables, we can avoid this problem.
			 */
			register entrypoint_t * entrypoint = (entrypoint_t*) os_stacks.stacks[os_threads[q].stack_id][OS_STACK_DWORD - 2];
			register void * data = (void *) os_stacks.stacks[os_threads[q].stack_id][OS_STACK_DWORD - 8];
			register void (*dispose)(int) = os_thread_dispose;

			// Start this thread
			uint32_t * thread_sp = os_threads[q].sp;
			__set_PSP(thread_sp);
			__set_CONTROL(0x03); 	// SPSEL = 1 | nPRIV = 1: use PSP and unpriveldged thread mode

			__ISB();

			asm volatile(
					"MOV LR, %[dispose]\n\t"
					"MOV R0, %[data]\n\t"
					"BX %[entrypoint]\n\t"
					:
					: [entrypoint] "r" (entrypoint), [dispose] "r" (dispose), [data] "r" (data)
					);

			// if thread we started here returns,
			// it returns here. 
			while (1);
		}
	}
}

static int os_stack_create()
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

static void os_stack_dispose(uint32_t stack_id)
{
	os_stacks.allocations &= ~stack_id;
}

static int os_thread_construct(int tid, entrypoint_t * entrypoint, void * data)
{
	if (tid < OS_THREADS)
	{
		if (os_threads[tid].state == THREAD_STATE_CREATED)
		{
			uint32_t stack_id = os_stack_create();
			if (stack_id != 0xFFFFFFFF)
			{
				os_threads[tid].stack_id = stack_id;
				os_threads[tid].sp = &os_stacks.stacks[stack_id][OS_STACK_DWORD - 16];

				os_stacks.stacks[stack_id][OS_STACK_DWORD - 8] = (uint32_t) data; // R0
				os_stacks.stacks[stack_id][OS_STACK_DWORD - 3] = (uint32_t) os_thread_dispose; // LR
				os_stacks.stacks[stack_id][OS_STACK_DWORD - 2] = (uint32_t) entrypoint; // PC
				os_stacks.stacks[stack_id][OS_STACK_DWORD - 1] = 0x01000000; // xPSR

				os_threads[tid].state = THREAD_STATE_READY;
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
	sched_tick_increment = xms * 1000;
	/* div8 per ST, stays compatible with M3/M4 parts, well done ST */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	/* clear counter so it starts right away */
	STK_CVR = 0;

	systick_set_reload(4000000);//rcc_ahb_frequency / 8 / 1000 * xms);
	systick_counter_enable();
	systick_interrupt_enable();
}

static int __os_thread_create(const struct OS_process_t * process, entrypoint_t * entrypoint, void * data, uint8_t priority)
{
	uint8_t thread_id = os_thread_alloc(process, priority);
	os_thread_construct(thread_id, entrypoint, NULL);
	return thread_id;
}
/** Syscall handling thread_create()
 * Creates new thread inside current process using specified entrypoint.
 */
int os_thread_create(entrypoint_t * entrypoint, void * data, uint8_t priority)
{
	uint8_t process_id = os_get_current_process();
	return __os_thread_create(&processes[process_id], entrypoint, data, priority);
}

	
static int os_thread_alloc(const struct OS_process_t * process, uint8_t priority)
{
	for (int q = 0; q < OS_THREADS; ++q)
	{
		if (os_threads[q].state == THREAD_STATE_EMPTY)
		{
			os_threads[q].stack_id = OS_TASK_NO_STACK;
			os_threads[q].process = process;
			os_threads[q].sp = (uint32_t *) ~0;
			os_threads[q].state = THREAD_STATE_CREATED;
			os_threads[q].priority = priority;
			return q;
		}
	}
	return ~0;
}

int os_thread_join(uint8_t thread_id)
{
	if (thread_id < OS_THREADS)
	{
		if (os_threads[thread_id].state == THREAD_STATE_FINISHED)
		{
			int rv = os_threads[thread_id].exit_status;
			os_threads[thread_id].state = THREAD_STATE_EMPTY;
			return rv;
		}
		else
		{
			uint8_t current_thread_id = os_get_current_thread();
			os_threads[current_thread_id].state = THREAD_STATE_BLOCKED_JOINING;
			os_threads[current_thread_id].block_object = thread_id;
		}
	}
	return E_INVALID;
}

int os_thread_exit(int status)
{
	uint8_t thread_id = os_get_current_thread();
	if (os_threads[thread_id].state == THREAD_STATE_RUNNING)
	{
		os_threads[thread_id].state = THREAD_STATE_FINISHED;
		os_threads[thread_id].exit_status = status;

		os_stack_dispose(os_threads[thread_id].stack_id);
		os_threads[thread_id].stack_id = OS_TASK_NO_STACK;
		os_threads[thread_id].sp = (uint32_t *) ~0;
		os_sched_yield();
	}
	return 0;
}

int os_thread_stop(uint8_t thread)
{
	if (thread < OS_THREADS)
	{
		if (os_threads[thread].state == THREAD_STATE_READY ||
				os_threads[thread].state == THREAD_STATE_RUNNING)
		{
			os_threads[thread].state = THREAD_STATE_STOPPED;
			if (thread == os_get_current_thread())
			{
				os_sched_yield();
				return 0;
			}
		}
		else
		{
			return E_NOTAVAIL;
		}
	}
	return E_INVALID;
}

int os_thread_continue(uint8_t thread)
{
	if (thread < OS_THREADS)
	{
		if (os_threads[thread].state == THREAD_STATE_STOPPED)
		{
			os_threads[thread].state = THREAD_STATE_READY;
			os_sched_yield();
			return 0;
		}
		else
		{
			return E_NOTAVAIL;
		}
	}
	return E_INVALID;
}
