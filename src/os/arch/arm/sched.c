/** @defgroup os_sched Kernel scheduler
 *
 * @ingroup os_kernel
 *
 * @brief Kernel scheduler internals
 *
 */

/** @ingroup os_sched
 * @{
 */
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched/stack.h>
#include <cmrx/os/timer.h>
#include <arch/memory.h>
#include <arch/cortex.h>
#include <string.h>

#ifdef TESTING
#define STATIC
#else
#define STATIC static
#endif

#include <cmrx/assert.h>

int __os_process_create(Process_t process_id, const struct OS_process_definition_t * definition);
int __os_thread_create(Process_t process, entrypoint_t * entrypoint, void * data, uint8_t priority);
int os_thread_alloc(Process_t process, uint8_t priority);
void os_thread_dispose(int arg0);
__attribute__((noreturn)) int os_idle_thread(void * data);
bool os_get_next_thread(uint8_t current_thread, uint8_t * next_thread);
int os_stack_create();
unsigned long * os_stack_get(int stack_id);
struct OS_thread_t * os_thread_get(Thread_t thread_id);
void os_sched_timed_event(void);

/** Make thread runnable.
 *
 * This function will take previously allocated thread and will construct it's
 * internal state, so that it is runnable. This includes stack allocation and
 * filling in values, so that thread can be scheduled and run.
 * @param tid Thread ID of thread to be constructed
 * @param entrypoint pointer to thread entrypoint function
 * @param data pointer to thread data. pass NULL pointer if no thread data is used
 * @returns E_OK if thread was constructed, E_OUT_OF_STACKS if there is no free stack
 * available and E_TASK_RUNNING if thread is not in state suitable for construction
 * (either slot is free, or already constructed).
 */
int os_thread_construct(int tid, entrypoint_t * entrypoint, void * data)
{
	if (tid < OS_THREADS)
	{
		if (os_threads[tid].state == THREAD_STATE_CREATED)
		{
			uint32_t stack_id = os_stack_create();
			if (stack_id != 0xFFFFFFFF)
			{
				os_threads[tid].stack_id = stack_id;
                unsigned long * stack = os_stack_get(stack_id);
//				os_threads[tid].sp = &os_stacks.stacks[stack_id][OS_STACK_DWORD - 16];
                os_threads[tid].sp = &stack[OS_STACK_DWORD - 16];

				stack[OS_STACK_DWORD - 8] = (unsigned long) data; // R0
				stack[OS_STACK_DWORD - 3] = (unsigned long) os_thread_dispose; // LR
				stack[OS_STACK_DWORD - 2] = (unsigned long) entrypoint; // PC
				stack[OS_STACK_DWORD - 1] = 0x01000000; // xPSR

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

/** Create process using process definition.
 * Takes process definition and initializes MPU regions for process out of it.
 * @param process_id ID of process to be initialized
 * @param definition process definition. This is constructed at compile time using OS_APPLICATION macros
 * @returns E_OK if process was contructed properly, E_INVALID if process ID is already used or
 * if process definition contains invalid section boundaries. E_OUT_OF_RANGE is returned if process ID
 * requested is out of limits given by the size of process table.
 */
int __os_process_create(Process_t process_id, const struct OS_process_definition_t * definition)
{
	if (process_id >= OS_PROCESSES)
	{
		return E_OUT_OF_RANGE;
	}
	
	if (os_processes[process_id].definition != NULL)
	{
		return E_INVALID;
	}

	os_processes[process_id].definition = definition;
	for (int q = 0; q < OS_TASK_MPU_REGIONS; ++q)
	{
		unsigned reg_size = (uint8_t *) definition->mpu_regions[q].end - (uint8_t *) definition->mpu_regions[q].start;
		int rv = mpu_configure_region(q, definition->mpu_regions[q].start, reg_size, MPU_RW, &os_processes[process_id].mpu[q]._MPU_RBAR, &os_processes[process_id].mpu[q]._MPU_RASR);
		if (rv != E_OK)
		{
			os_processes[process_id].definition = NULL;
			return E_INVALID;
		}
	}
	return E_OK;
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
	for (int q = 0; q < OS_THREADS; ++q)
	{
		if (os_threads[q].state == THREAD_STATE_EMPTY)
		{
			memset(&os_threads[q], 0, sizeof(os_threads[q]));
			os_threads[q].stack_id = OS_TASK_NO_STACK;
			os_threads[q].process_id = process;
			os_threads[q].sp = (unsigned long *) ~0;
			os_threads[q].state = THREAD_STATE_CREATED;
			os_threads[q].signals = 0;
			os_threads[q].signal_handler = NULL;
			os_threads[q].priority = priority;
			return q;
		}
	}
	return ~0;
}

__attribute__((naked,noreturn)) void __os_boot_thread(Thread_t boot_thread)
{
    // Start this thread
    // We are adding 8 here, because normally pend_sv_handler would be reading 8 general 
    // purpose registers here. But there is nothing useful there, so we simply skip it.
    // Code belog then restores what would normally be restored by return from handler.
    struct OS_thread_t * thread = os_thread_get(boot_thread);
    unsigned long * thread_sp = thread->sp + 8;
    __set_PSP((uint32_t) thread_sp);
    __set_CONTROL(0x03); 	// SPSEL = 1 | nPRIV = 1: use PSP and unpriveldged thread mode

    __ISB();

    __ISR_return();

}

/** @} */
