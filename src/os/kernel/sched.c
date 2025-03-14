/** @addtogroup os_sched
 * @{
 */
#include <cmrx/defines.h>
#include <stdint.h>
#include <conf/kernel.h>
#include <arch/corelocal.h>
#include "notify.h"
#include "sched.h"
#include "runtime.h"
#include "sched.h"
#include "txn.h"
#include <stdbool.h>
#include <cmrx/assert.h>
#include "arch/static.h"
#include "arch/sched.h"
#include "arch/context.h"
#include "context.h"
#include "arch/mpu.h"
#include "timer.h"
#include <cmrx/sys/syscalls.h>
#include <cmrx/clock.h>
#include <string.h>

#define STACK_ALIGN 

typedef uint8_t Thread_t;

/** List of active threads. */
struct OS_thread_t os_threads[OS_THREADS];

/** List of active processes. */
struct OS_process_t os_processes[OS_PROCESSES];

/** CPU scheduling thread IDs */
struct OS_core_state_t core[OS_NUM_CORES];

#define PRIORITY_INVALID    0x1FFU
#define PRIORITY_MAX        0xFFU
#define STACK_INVALID       0xFFFFFFFFU

/// @cond IGNORE
__attribute__((aligned(OS_STACK_SIZE)))
/// @endcond
/** Thread stacks */
struct OS_stack_t os_stacks;

/** Amount of real time advance per one scheduler tick. */
extern uint32_t sched_tick_increment;

/** Current scheduler real time */
static uint32_t sched_microtime = 0;

/* Forward declaration. */
int os_thread_alloc(Process_t process, uint8_t priority);
int __os_thread_create(Process_t process, entrypoint_t * entrypoint, void * data, uint8_t priority, uint8_t core);

/** Obtain next thread to run.
 *
 * This function performs thread list lookup. It searches for thread, which 
 * is in ready state and has highest (numerically lowest) priority.
 *
 * @param current_thread thread which is currently running
 * @param next_thread pointer to variable where next thread ID will be stored
 * @returns true if any runnable thread (different than current) was found, false
 * otherwise.
 */
__attribute__((noinline)) bool os_get_next_thread(uint8_t current_thread, uint8_t * next_thread)
{
	uint16_t best_prio = PRIORITY_INVALID;
    uint8_t candidate_thread = 0xFF;
    uint8_t thread = (current_thread + 1) % OS_THREADS;
    uint8_t core_id = coreid();

	uint8_t loops = OS_THREADS;

	do {
		if ((os_threads[thread].state == THREAD_STATE_READY 
				|| os_threads[thread].state == THREAD_STATE_RUNNING)
                && os_threads[thread].core_id == core_id)
		{
			/* Schedule different thread only if it has lower numerical value of priority
			 * than one we've already found. This should enforce prioritized round robin
			 * scheduling
			 */
			if (os_threads[thread].priority < best_prio)
			{
				candidate_thread = thread;
				best_prio = os_threads[thread].priority;
			}
		}

		thread = (thread + 1) % OS_THREADS;
	} while (loops--);

	ASSERT(loops > 0);
    ASSERT(candidate_thread < OS_THREADS);

    *next_thread = candidate_thread;

	if (best_prio <= PRIORITY_MAX && candidate_thread != current_thread)
	{
		return true;
	}

	return false;
}

int os_sched_yield(void)
{
	uint8_t candidate_thread;

//	os_sched_timed_event();
    uint8_t txn_id = os_txn_start();

    struct OS_core_state_t * core_state = &core[coreid()];

	if (os_get_next_thread(core_state->thread_current, &candidate_thread))
	{
        if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK) {
            core_state->thread_prev = core_state->thread_current;
            core_state->thread_next = candidate_thread;
            schedule_context_switch(core_state->thread_current, candidate_thread);
            os_txn_done();
        }
 	}
 	else
    {
        os_request_context_switch(false);
    }

	return 0;
}

/*
void os_sched_timed_event(void)
{
	unsigned delay;
	if (os_schedule_timer(&delay))
	{
		sched_timer_event = sched_microtime + delay;
		sched_timer_event_enabled = true;
	}
	else
	{
		sched_timer_event_enabled = false;
	}
}
*/

/* was: Handler of periodic systick handler.
 *
 * This function is called periodically based on how is systick timer set up
 * by startup code. Normally it performs execution of timed events and switches
 * currently running thread, if there is any other thread to be run.
 */
long os_sched_timing_callback(long delay_us)
{
//	ASSERT(__get_LR() == (void *) 0xFFFFFFFD);
	ASSERT(os_threads[core[coreid()].thread_current].state == THREAD_STATE_RUNNING);
/*	unsigned long * psp;
	psp = (uint32_t *) __get_PSP();
	ASSERT(&os_stacks.stacks[0][0] <= psp && psp <= &os_stacks.stacks[OS_STACKS][OS_STACK_DWORD]);*/

//	was: sched_microtime += sched_tick_increment;
    sched_microtime += delay_us;

/*	if (sched_timer_event_enabled &&
			sched_timer_event == sched_microtime)
	{*/
	os_run_timer(sched_microtime);

//}
	os_sched_yield();

//	os_sched_timed_event();
	unsigned rt = 0;
	for (int q = 0; q < OS_THREADS; ++q)
	{
		if (os_threads[q].state == THREAD_STATE_RUNNING && os_threads[q].core_id == coreid())
			rt++;
	}

	ASSERT(rt == 1);
	ASSERT(os_threads[core[coreid()].thread_current].state == THREAD_STATE_RUNNING);
/*	psp = (uint32_t *) __get_PSP();
	ASSERT(&os_stacks.stacks[0][0] <= psp && psp <= &os_stacks.stacks[OS_STACKS][OS_STACK_DWORD]);
	__DSB();
	__ISB();*/
    return 1;
}

uint8_t os_get_current_thread(void)
{
	return core[coreid()].thread_current;
}

void os_set_current_thread(Thread_t new_thread)
{
	core[coreid()].thread_current = new_thread;
}

uint8_t os_get_current_stack(void)
{
	return os_threads[core[coreid()].thread_current].stack_id;
}

uint8_t os_get_current_process(void)
{
	return os_threads[core[coreid()].thread_current].process_id;
}

uint32_t os_get_micro_time(void)
{
	return sched_microtime;
}

/// @cond IGNORE 
__attribute__((noreturn)) 
/// @endcond
/** CMRX idle thread.
 *
 * This thread runs whenever no other CMRX thread is ready to be run.
 * It does nothing useful. */
int os_idle_thread(void * data)
{
    (void) data;
	while (1);
}

/** Find free stack slot and allocate it.
 * @return If there is at least one free stack slot, then return it's ID. If no free stack
 * is available, return STACK_INVALID constant.
 */
int os_stack_create()
{
	uint32_t stack_mask = 1;
    int rv = STACK_INVALID;

    uint8_t txn_id = os_txn_start();

	for(int q = 0; q < OS_STACKS; ++q)
	{
		if ((os_stacks.allocations & stack_mask) == 0)
		{
            if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK) {
    			os_stacks.allocations |= stack_mask;
	    		rv = q;
                os_txn_done();
                break;
            } else {
                // Transaction aborted, restart search
                txn_id = os_txn_start();
                q = -1;
                continue;
            }
		}
		stack_mask *= 2;
	}

	return rv;
}

uint32_t * os_stack_get(int stack_id)
{
    return os_stacks.stacks[stack_id];
}

/** Release stack slot.
 * @param stack_id Stack slot which should be released.
 */
void os_stack_dispose(uint32_t stack_id)
{
	if (stack_id < OS_STACKS)
	{
        os_txn_start_commit();
		os_stacks.allocations &= ~(1 << stack_id);
        os_txn_done();
	}
}

int os_thread_exit(int status)
{
	uint8_t thread_id = os_get_current_thread();
	return os_thread_kill(thread_id, status);
}

int os_thread_kill(uint8_t thread_id, int status)
{
	if (thread_id >= OS_THREADS)
	{
		return E_INVALID;
	}

    struct OS_thread_t * const thread = &os_threads[thread_id];
	if (thread->state != THREAD_STATE_EMPTY
			&& thread->state != THREAD_STATE_FINISHED
			)
	{
        os_txn_start_commit();
		thread->state = THREAD_STATE_FINISHED;
		thread->exit_status = status;
        uint32_t stack_id = thread->stack_id;
		thread->stack_id = OS_TASK_NO_STACK;
		thread->sp = (uint32_t *) ~0;
        os_txn_done();
		os_stack_dispose(stack_id);
		if (thread_id == os_get_current_thread())
		{
			os_sched_yield();
		}
		return 0;
	}
	return E_INVALID;
}

int os_thread_stop(uint8_t thread)
{
	if (thread < OS_THREADS)
	{
        while (1) 
        {
            Txn_t txn = os_txn_start();
            if (os_threads[thread].state == THREAD_STATE_READY ||
                    os_threads[thread].state == THREAD_STATE_RUNNING)
            {
                if (os_txn_commit(txn, TXN_READWRITE) == E_OK)
                {
                    os_threads[thread].state = THREAD_STATE_STOPPED;
                    os_txn_done();
                } else {
                    // Try again!
                    continue;
                }
                if (thread == os_get_current_thread())
                {
                    os_sched_yield();
                }
                return 0;
            }
            else
            {
                return E_NOTAVAIL;
            }
        }
	}
	return E_INVALID;
}

struct OS_thread_t * os_thread_by_id(Thread_t id)
{
    if (id < OS_THREADS)
    {
        return &os_threads[id];
    }

    return NULL;
}

int os_thread_set_ready(struct OS_thread_t * thread)
{
    thread->state = THREAD_STATE_READY;
    return 0;
}

int os_thread_continue(uint8_t thread)
{
	if (thread < OS_THREADS)
	{
        while (1)
        {
            Txn_t txn = os_txn_start();
            if (os_threads[thread].state == THREAD_STATE_STOPPED)
            {
                if (os_txn_commit(txn, TXN_READWRITE) == E_OK) 
                {
                    os_thread_set_ready(&os_threads[thread]);
                    os_txn_done();
                }
                os_sched_yield();
                return 0;
            }
            else
            {
                return E_NOTAVAIL;
            }
        }
	}
	return E_INVALID;
}

int os_setpriority(uint8_t priority)
{
    os_txn_start_commit();
	os_threads[os_get_current_thread()].priority = priority;
    os_txn_done();
	os_sched_yield();
	return 0;
}

static void cb_thread_join_notify(const void * object, Thread_t thread, int sleeper_id, Event_t event)
{
    struct OS_thread_t * dead_thread = (struct OS_thread_t *) object;
    if (os_threads[thread].state == THREAD_STATE_WAITING
        && os_threads[thread].wait_object == object)
    {
        if (event == EVT_DEFAULT)
        {
            os_set_syscall_return_value(thread, dead_thread->exit_status);
        }
    }
}

int os_thread_join(uint8_t thread_id)
{
	if (thread_id < OS_THREADS)
	{
        os_txn_start_commit();
        if (os_threads[thread_id].state == THREAD_STATE_FINISHED)
        {
            int rv = os_threads[thread_id].exit_status;
            os_threads[thread_id].state = THREAD_STATE_EMPTY;
            os_txn_done();
            return rv;
        }
        else
        {
            uint8_t current_thread_id = os_get_current_thread();
            os_txn_done();
            os_wait_for_object(&os_threads[current_thread_id], cb_thread_join_notify);
        }
	}
	return E_INVALID;
}

/** Full workflow needed to create a thread.
 *
 * This function is callable both from syscall and internally from kernel (during e.g. system startup)
 * and performs complete thread creation workflow.
 * @param process ID of process owning the thread. Process must already be existing.
 * @param entrypoint address of function which shall be executed as entrypoint into the thread
 * @param data address of data block which should be passed to entrypoint function as an argument
 * @param priority thread priority. Numerically lower values mean higher priorities
 * @param core core on which thread will be created
 * @return Non-negative values denote ID of thread just created, negative values mean errors.
 */
int __os_thread_create(Process_t process, entrypoint_t * entrypoint, void * data, uint8_t priority, uint8_t core)
{
	uint8_t thread_id = os_thread_alloc(process, priority);
    if (thread_id < OS_THREADS)
    {
    	os_thread_construct(thread_id, entrypoint, data, core);
    }
    ASSERT(thread_id < OS_THREADS);
	return thread_id;
}

int os_thread_construct(Thread_t tid, entrypoint_t * entrypoint, void * data, uint8_t core_id)
{
	if (tid < OS_THREADS)
	{
		if (os_threads[tid].state == THREAD_STATE_CREATED)
		{
            struct OS_thread_t * new_thread = &os_threads[tid];
			uint32_t stack_id = os_stack_create();
			if (stack_id != 0xFFFFFFFF)
			{
				new_thread->stack_id = stack_id;
                new_thread->sp = os_thread_populate_stack(stack_id, OS_STACK_DWORD, entrypoint, data);
                new_thread->core_id = core_id;

				new_thread->state = THREAD_STATE_READY;
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

/** Allocate thread entry in thread table.
 * Will allocate entry in thread table. Thread won't be runnable after
 * allocation, but thread ID will be reserved for it.
 * @param process ID of process owning the thread. Process must be existing already.
 * @param priority tread priority
 * @return Positive values denote thread ID reserved for new thread usable in further calls. Negative
 * value means that there was no free slot in thread table to allocate new thread.
 */
int os_thread_alloc(Process_t process, uint8_t priority)
{
    Txn_t txn = os_txn_start();
    for (int q = 0; q < OS_THREADS; ++q)
    {
        if (os_threads[q].state == THREAD_STATE_EMPTY)
        {
            if (os_txn_commit(txn, TXN_READWRITE) == E_OK)
            {
                struct OS_thread_t * new_thread = &os_threads[q];
                memset(new_thread, 0, sizeof(struct OS_thread_t));
                new_thread->stack_id = OS_TASK_NO_STACK;
                new_thread->process_id = process;
                new_thread->sp = (uint32_t *) ~0;
                new_thread->state = THREAD_STATE_CREATED;
                new_thread->signals = 0;
                new_thread->signal_handler = NULL;
                new_thread->priority = priority;
                os_txn_done();
                return q;
            }
            else
            {
                txn = os_txn_start();
                q = -1;
                continue;
            }
        }
    }
    return E_OUT_OF_THREADS;
}

/** Syscall handling thread_create()
 * Creates new thread inside current process using specified entrypoint.
 */
int os_thread_create(entrypoint_t * entrypoint, void * data, uint8_t priority)
{
	uint8_t process_id = os_get_current_process();
    uint8_t core_id = coreid();
	return __os_thread_create(process_id, entrypoint, data, priority, core_id);
}

#define KERNEL_STRUCTS_INITIALIZED_SIGNATURE    0x434D5258

void _os_start(uint8_t start_core)
{
    volatile Thread_t created_threads[8];
    volatile unsigned cursor = 0;
    for (int q = 0; q < 8; ++q) {
        created_threads[q] = 0xFF;
    }
    static uint32_t kernel_structs_initialized = 0;
	unsigned threads = static_init_thread_count(); 
	unsigned applications = static_init_process_count(); 
	const struct OS_process_definition_t * const app_definition = static_init_process_table();
	const struct OS_thread_create_t * const autostart_threads = static_init_thread_table();

    os_smp_lock();
    if (kernel_structs_initialized != KERNEL_STRUCTS_INITIALIZED_SIGNATURE)
    {
        kernel_structs_initialized = KERNEL_STRUCTS_INITIALIZED_SIGNATURE;
        memset(&os_threads, 0, sizeof(os_threads));
        os_timer_init();
        os_notify_init();
    }
    os_smp_unlock();

	for (unsigned q = 0; q < applications; ++q)
	{
		os_process_create(q, &app_definition[q]);
	}

	for (unsigned q = 0; q < threads; ++q)
	{
        const struct OS_thread_create_t * const autostarted_thread = &autostart_threads[q];
        if (autostarted_thread->core != start_core)
        {
            continue;
        }

		Process_t process_id = (autostarted_thread->process - app_definition);
        created_threads[cursor] = __os_thread_create(process_id, autostarted_thread->entrypoint, autostarted_thread->data, autostarted_thread->priority, start_core);
        ASSERT(created_threads[cursor] != E_OUT_OF_THREADS);
        cursor++;
	}

	created_threads[cursor] = __os_thread_create((uint32_t) 0, os_idle_thread, NULL, 0xFF, start_core);
    ASSERT(created_threads[cursor] != E_OUT_OF_THREADS);
    cursor++;

	uint8_t startup_thread;

	if (os_get_next_thread(0xFF, &startup_thread))
	{
		core[start_core].thread_current = startup_thread;
		Process_t startup_process = os_threads[startup_thread].process_id;
		os_threads[startup_thread].state = THREAD_STATE_RUNNING;

        os_memory_protection_start();
		// Flash - RX
		// At this state thread is not hosted anywhere
		mpu_restore((const MPU_State *) &os_processes[startup_process].mpu, (const MPU_State *) &os_processes[startup_process].mpu);

		// Configure stack access for incoming thread
		if (mpu_init_stack(startup_thread) != E_OK)
		{
			ASSERT(0);
		}
	
        // Fire up timer, which timing provider uses to tick the kernel
        timing_provider_schedule(1);

        os_boot_thread(startup_thread);
		// if thread we started here returns,
		// it returns here. 
	}
}

uint32_t os_shutdown(void)
{
    // For disabling interrupts
    os_core_lock();

    // Disable timing provider
    timing_provider_schedule(0);

    os_memory_protection_stop();

    os_kernel_shutdown();
}

struct OS_thread_t * os_thread_get(Thread_t thread_id)
{
    return &os_threads[thread_id];
}

int os_thread_migrate(uint8_t thread_id, int target_core)
{
    uint8_t current_core = coreid();
    if (thread_id < OS_THREADS)
    {
        Txn_t txn = os_txn_start();
        struct OS_thread_t * thread = os_thread_get(thread_id);
        if (thread->core_id != current_core || thread->state != THREAD_STATE_STOPPED)
        {
            return E_INVALID;
        }
        else 
        {
            if (os_txn_commit(txn, TXN_READONLY) == E_OK)
            {
                // This stuff is not actually fully implemented yet
                // Thread needs to be picked up by the receiving core.
                // Receiving core needs to be pinged about the arriving thread.
                thread->state = THREAD_STATE_MIGRATING;
                thread->core_id = target_core;
                return E_NOTAVAIL;
            }
            else
            {
                // This can potentically cause problems if transaction was interrupted for some
                // unrelated read-write transaction. While it is not clear yet if this function
                // is actually useful, leave this as is. Can be fixed later.
                return E_INVALID;
            }
        }
    }
    return E_OUT_OF_RANGE;
}

/** @} */
