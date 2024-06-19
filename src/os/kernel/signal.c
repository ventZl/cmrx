/** @defgroup os_signal Signal handling
 * @ingroup os
 * @{ 
 */
#include <cmrx/assert.h>
#include <cmrx/ipc/signal.h>
#include <cmrx/os/sched.h>

int os_signal(int signo, void (*sighandler)(int))
{
    (void) signo;
    /* @todo: what to do with signo? */
	uint8_t thread_id = os_get_current_thread();
	ASSERT(thread_id < OS_THREADS);

	os_threads[thread_id].signal_handler = sighandler;
	return 0;
}

int os_kill(uint8_t thread_id, uint8_t signal_id)
{
	ASSERT(thread_id < OS_THREADS);
    struct OS_thread_t * thread = &os_threads[thread_id];
	if (thread->state == THREAD_STATE_READY 
			|| thread->state == THREAD_STATE_RUNNING
			|| thread->state == THREAD_STATE_STOPPED
			)
	{
		if (signal_id < 32)
		{
			thread->signals |= 1 << signal_id;

			if (thread->state == THREAD_STATE_STOPPED)
			{
				os_thread_continue(thread_id);
			}
			return 0;
		}
		else
		{
			if (signal_id == SIGSTOP)
			{
				return os_thread_stop(thread_id);
			} 
			else if (signal_id == SIGCONT)
			{
				return os_thread_continue(thread_id);
			} 
			else if (signal_id == SIGKILL)
			{
				return os_thread_kill(thread_id, -SIGCONT);
			}
			else if (signal_id == SIGSEGV)
			{
				return os_thread_kill(thread_id, -SIGSEGV);
			}
			else
			{
				// Asserted here? calling thread asked for unsupported signal.
				ASSERT(0);
				return E_INVALID;
			}
		}
	}
	return E_INVALID;
}

/** @} */
