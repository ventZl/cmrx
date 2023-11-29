/** @defgroup api_thread Threading functions
 *
 * @ingroup api
 *
 * Functions providing support for manipulation of system execution state. It is possible
 * to create new threads, wait for other threads to finish or finish currently running thread.
 *
 * Threads are defined by their entry function, which treated similarly to main() function
 * of a C program. If thread entry function returns, then it's return value is used as 
 * thread return status. This is not interpreted by kernel in any way, but it is available 
 * for others.
 */

/** @ingroup api_thread
 * @{
 */
#pragma once

#include <arch/sysenter.h>

/** Return current thread ID.
 *
 * @returns thread ID of currently running thread.
 */
__SYSCALL int get_tid();

/** Give up processor.
 *
 * Call to this method cause thread switch. If thread switch occurs, or not, depends
 * on how thread priorities are configured. If there is no other thread ready at
 * equal or higher priority than currently running thread, then switch won't occurr.
 * @returns 0. Mostly.
 */
__SYSCALL int sched_yield();

/** Create new thread.
 *
 * Creates new thread and prepares it for scheduling. This routine can be used for 
 * on-demand thread creation. New thread will be bound to current process (the one
 * which owns currently running thread). You can set up thread entrypoint, pass it
 * some user data and opt for thread priority.
 * @param entrypoint function, which will be called upon thread startup to run the
 * thread
 * @param data user-defined data passed to the entrypoint as first argument
 * @priority priority of newly created thread. Lower numbers mean higher priorities.
 * Thread with priority 0 has realtime priority, thread with priority 255 is an idle
 * thread. Note that there already is one idle thread and if you create another, 
 * then outcome most probably won't be as expected. Use priority 254 for custom
 * idle threads.
 * @returns non-negative numbers carrying thread ID of newly created thread or 
 * negative numbers to signal error.
 */
__SYSCALL int thread_create(int (*entrypoint)(void *), void * data, uint8_t priority);

/** Wait for other thread to finish.
 * 
 * This function will block calling thread until other thread quits.
 * @param thread thread ID of other threads, which this thread wants to fair for
 * @param status place for return value from other thread to be written
 * @returns 0 on success (other thread quit and status value is written), error
 * code otherwise.
 */
__SYSCALL int thread_join(int thread);

/** Terminate currently running thread.
 *
 * This function will explicitly terminate currently running thread.
 * Another way to implicitly terminate running thread is to return from thread
 * entry function. Thread will be terminated automatically using return value as
 * thread exit status in a way similar to how return from main() is used as process
 * exit status.
 * @param status thread exit status
 * @return You don't want this function to return.
 */
__SYSCALL int thread_exit(int status);

/** Change thread priority.
 *
 * Allows to change thread priority. Currently it is only possible to change priority
 * of currently running thread. Thread priorities go from highest (numeric value of 0)
 * to idle thread priority (numeric value of 255). It is not recommended to assign priority
 * of 255 to any thread as this priority is used by kernel's idle thread.
 * @param priority new priority 
 */

__SYSCALL int setpriority(uint8_t priority);

/** @} */

