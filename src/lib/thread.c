/** @ingroup api_thread
 * @{
 */

#include <cmrx/ipc/thread.h>
#include <cmrx/os/syscalls.h>

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
	__SVC(SYSCALL_THREAD_CREATE);
}

__SYSCALL int thread_join(int thread)
{
    (void) thread;
	__SVC(SYSCALL_THREAD_JOIN);
}

__SYSCALL int thread_exit(int status)
{
    (void) status;
	__SVC(SYSCALL_THREAD_EXIT);
}

__SYSCALL void usleep(uint32_t microseconds)
{
    (void) microseconds;
	__SVC(SYSCALL_USLEEP);
}

__SYSCALL void setitimer(uint32_t microseconds)
{
    (void) microseconds;
	__SVC(SYSCALL_SETITIMER);
}

__SYSCALL int setpriority(uint8_t priority)
{
    (void) priority;
	__SVC(SYSCALL_SETPRIORITY);
}

/** @} */
