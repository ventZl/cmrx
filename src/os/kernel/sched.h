#pragma once

#include "runtime.h"
#include <stdbool.h>
#include <arch/corelocal.h>

/** @defgroup os_sched Thread scheduling
 * @ingroup os
 * Kernel primitives for manipulating threads.
 * @{
 */

extern struct OS_core_state_t core[OS_NUM_CORES];

/** Kernel callback for timing provider.
 * 
 * Kernel entrypoint for timed events. Kernel tells the timing provider, what is the delay 
 * before the next call, whenever this function is called. Timing provider shall then wait
 * for given amount of time and then call this callback again. If the delay is 0, then 
 * timing provider can shutdown itself as there is no expected timed wakeup.
 * @param [in] delay_us the actual amount of microseconds which happened since the last wakeup
 * @returns amount of microseconds, which shall pass before next wakeup will happen. If this 
 * value is zero, then no next wakeup shall happen.
 */
long os_sched_timing_callback(long delay_us);

/** Kernel implementation of get_pid.
 *
 * @returns Current process ID. This is actually an offset in process table.
 */
uint8_t os_get_current_process(void);

/** Kernel implementation of get_tid() syscall.
 *
 * @returns Current thread ID. This is actually an offset in thread table.
 */
uint8_t os_get_current_thread(void);

/** Kernel internal function to *override* current thread.
 *
 * DANGEROUS!!!
 */
void os_set_current_thread(Thread_t new_thread);

/** Get ID of stack used by current thread.
 *
 * @return Current active stack ID. This is actually an offset in stack table.
 */
uint8_t os_get_current_stack(void);

/** Get amount of microseconds elapsed since scheduler start.
 *
 * @returns internal kernel soft timer. This gets updated upon each systick handler
 * call. It may jitter a bit. Wraps after about 4 million seconds.
 */
uint32_t os_get_micro_time(void);

/** Kernel implementation of sched_yield() syscall.
 *
 * Causes scheduler to consider another task to be ran.
 */
int os_sched_yield(void);

/** Start up scheduler.
 *
 * This function populates thread table based on thread autostart macro use.
 * It also creates idle thread with priority 255 and starts scheduler. It never 
 * returns until you have very bad day.
 * @param [in] start_core number of core for which the kernel is started
 */
void _os_start(uint8_t start_core);

/** Kernel implementation of thread_create() syscall.
 *
 */
int os_thread_create(entrypoint_t * entrypoint, void * data, uint8_t priority);

/** Kernel implementation of thread_join() syscall.
 *
 */
int os_thread_join(uint8_t thread_id);

/** Kernel implementation of thread_exit() syscall.
 *
 */
int os_thread_exit(int status);

/** Kernel implementation of thread_stop() syscall.
 *
 */
int os_thread_stop(uint8_t thread_id);

/** Kernel implementation of thread_continue() syscall.
 *
 */
int os_thread_continue(uint8_t thread_id);

/** Kernel way to kill an arbitrary thread.
 *
 * This call terminates any thread currently existing. There is no syscall for this right now.
 * @param thread_id ID of thread to be terminated. May even be thread currently running.
 * @param status thread exit status
 * @return 0 if operation succeeded, error number otherwise
 */
int os_thread_kill(uint8_t thread_id, int status);

/** Kernel implementation of setpriority() syscall.
 */

int os_setpriority(uint8_t priority);

/** Get address of stack.
 * @param stack_id ID of stack
 * @returns base address of stack
 */
uint32_t * os_stack_get(int stack_id);

/** Get thread descriptor.
 * @param thread_id ID of thread
 * @returns address of thread description strcture or NULL pointer if thread_id out of range.
 */
struct OS_thread_t * os_thread_get(Thread_t thread_id);

/** Make thread runnable.
 *
 * This function will take previously allocated thread and will construct it's
 * internal state, so that it is runnable. This includes stack allocation and
 * filling in values, so that thread can be scheduled and run.
 * @param tid Thread ID of thread to be constructed
 * @param entrypoint pointer to thread entrypoint function
 * @param data pointer to thread data. pass NULL pointer if no thread data is used
 * @param core ID of core the thread should run at
 * @returns E_OK if thread was constructed, E_OUT_OF_STACKS if there is no free stack
 * available and E_TASK_RUNNING if thread is not in state suitable for construction
 * (either slot is free, or already constructed).
 */
int os_thread_construct(Thread_t tid, entrypoint_t * entrypoint, void * data, uint8_t core);

/** Alias to thread_exit.
 * This is in fact the same function as @ref thread_exit. The only difference is 
 * that if for whatever reason syscall to os_thread_exit() will fail, this asserts.
 */
void os_thread_dispose(void);

/** Migrate thread between CPU cores.
 * This function takes existing thread which is bound to some core 
 * and moved it over to scheduler queue of another core. In order for this call
 * to be successful, the thread must already be stopped. If thread is woken up 
 * by e.g. signal arrived from interrupt service handler via isr_kill() while
 * this call is in progress then the call will fail.
 * @param thread_id ID of thread to be migrated
 * @param target_core ID of core where the thread should be migrated.
 * @returns E_OK if thread was migrated; E_INVALID if thread is not stopped or call is 
 * not made from the core at which the thread is currently running.
 */
int os_thread_migrate(uint8_t thread_id, int target_core);

/** Kernel implementation of the shutdown() syscall.
 */
uint32_t os_shutdown();

/** @} */
