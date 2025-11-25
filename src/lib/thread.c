#include <cmrx/ipc/thread.h>
#include <cmrx/sys/syscalls.h>
#include <cmrx/api.h>
#include <cmrx/assert.h>

__SYSCALL int sched_yield()
{
	__SVC(SYSCALL_SCHED_YIELD);
}

__SYSCALL int get_tid()
{
	__SVC(SYSCALL_GET_TID);
}

__SYSCALL int thread_create(int (*entrypoint)(void *), void * data, uint8_t priority)
{
    (void) entrypoint;
    (void) data;
    (void) priority;
	__SVC(SYSCALL_THREAD_CREATE, entrypoint, data, priority);
}

__SYSCALL int thread_join(int thread)
{
    (void) thread;
	__SVC(SYSCALL_THREAD_JOIN, thread);
}

__SYSCALL int thread_exit(int status)
{
    (void) status;
	__SVC(SYSCALL_THREAD_EXIT, status);
}

__SYSCALL int setpriority(uint8_t priority)
{
    (void) priority;
	__SVC(SYSCALL_SETPRIORITY, priority);
}

/** @ingroup api_thread
 * @{
 */

/** Internal function, which disposes of thread which called it.
 *
 * This function is injected into stack (value of LR of thread entrypoint)
 * of each thread, so if thread entry function returns, the thread is disposed
 * automatically. It causes thread to exit with value returned by thread
 * entrypoint to be recorded as thread return value.
 * @param arg0 value returned by thread entrypoint
 */
int os_thread_dispose(int arg0)
{
    (void) arg0;
	// Do not place anything here. It will clobber R0 value!
	//
	// Normally, call to thread_exit would be here. But as we know that the way which
	// led to os_thread_dispose being called results into R0 holding arg0, we may call
	// syscall directly.
	__SVC(SYSCALL_THREAD_EXIT, arg0);
	//thread_exit(arg0);
	ASSERT(0);
	// this should be called when thread returns
}

/** @} */
