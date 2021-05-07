/** @defgroup os_sched Threads
 *
 * @ingroup os
 * @{
 */
#pragma once

#include <cmrx/os/runtime.h>

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
 */
void os_start();

/** Configures systick timer.
 *
 * Configures systick timer to cause sys_tick_handler to be called periodically.
 * @param xms amount of milliseconds between calls.
 */
void systick_setup(int xms);

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

/** @} */
