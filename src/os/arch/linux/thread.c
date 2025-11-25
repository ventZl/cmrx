#include "thread.h"

#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <kernel/sched.h>

/** Current CMRX thread ID
 *
 * This is a thread-local variable, so each thread has its own copy
 * It contains CMRX thread ID of the current Linux thread
 */

__thread int current_thread_id = -1;

void thread_suspend_execution(bool initial)
{
    assert(current_thread_id != -1);
    assert(os_threads[current_thread_id].arch.is_suspended == 0);

    // At this point the thread was suspended!
    os_threads[current_thread_id].arch.is_suspended = 1;

    char byte;
    // This is the synchronization point, thread is forced to wait
    int rv;
    do {
        rv = read(os_threads[current_thread_id].arch.block_pipe[0], &byte, 1);
    } while (rv != 1);

    if (!initial)
    {
        assert(os_threads[current_thread_id].state == THREAD_STATE_RUNNING);
    }

    // At this point the thread was resumed!
    os_threads[current_thread_id].arch.is_suspended = 0;
}

/** Unblocks the execution of another thread.
 *
 * Unblocks execution of another thread. The thread in question must have been
 * blocked prior this unblock.
 * @param thread_id ID of thread to be unblocked. Must be different than the current thread
 */
void thread_resume_execution(Thread_t thread_id)
{
    assert(thread_id < OS_THREADS);
    assert(current_thread_id != -1);
    assert(current_thread_id != thread_id);
    assert(os_threads[thread_id].arch.is_suspended == 1);

    // TODO: Can this be called from non-CMRX-thread context?
    // As of now assume that this is a plausible scenario

    char byte = 0;
    write(os_threads[thread_id].arch.block_pipe[1], &byte, 1);
}

void enter_system_call()
{
    pthread_kill(pthread_self(), SIGURG);
}

void request_pending_service()
{
    pthread_kill(pthread_self(), SIGUSR1);
}
