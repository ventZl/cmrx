#pragma once

#include <kernel/runtime.h>

/** Current CMRX thread ID
 *
 * This is a thread-local variable, so each thread has its own copy
 * It contains CMRX thread ID of the current Linux thread
 */
extern __thread int current_thread_id;

/** Blocks current thread execution.
 *
 * Must be called from CMRX context thread.
 * @param initial if true then initial thread suspension is performed. Initial thread suspension serves the purpose to suspend the POSIX thread execution after it was created. Here some condition checks are omitted.
 */
void thread_suspend_execution(bool initial);

/** Unblocks the execution of another thread.
 *
 * Unblocks execution of another thread. The thread in question must have been
 * blocked prior this unblock.
 * @param thread_id ID of thread to be unblocked. Must be different than the current thread
 */
void thread_resume_execution(Thread_t thread_id);

/** Transfer control from userspace to kernel space.
 */
void enter_system_call();

/** Transfer control to pending service mechanism.
 */
void request_pending_service();
