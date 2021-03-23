#include <cmrx/os/signal.h>
#include <cmrx/os/sched.h>
#include <cmrx/assert.h>
#include <cmrx/os/runtime.h>
#include <cmrx/ipc/signal.h>
#include <cmrx/defines.h>
#include <cmrx/os/syscall.h>
#include <cmrx/intrinsics.h>

int os_signal(int signo, void (*sighandler)(int))
{
	uint8_t thread_id = os_get_current_thread();
	ASSERT(thread_id < OS_THREADS);

	os_threads[thread_id].signal_handler = sighandler;
	return 0;
}

/** Perform signal delivery in thread's userspace.
 * This "function" is ever only called from os_deliver_signal(). It stores
 * original values of LR and R0 - R3 registers and PC onto stack. 
 * @param sighandler address of signal handler function. Guarranteed to be non-NULL. Yet not to be valid.
 * @param signal_mask Bitmask of activated (pending) signals, which are catchable by the application.
 */
__attribute__((naked)) static void os_fire_signal(uint32_t signal_mask, void *sighandler(void))
{

	asm volatile(
			"BLX %[sighandler]\n\t"
			"POP { lr }\n\t"
			"POP { r0 - r3, pc }\n\t"
			:
			: [sighandler] "r" (sighandler)
			);
}

static void os_deliver_signal(uint8_t thread_id, uint32_t signals)
{
	ASSERT(thread_id < OS_THREADS);
	ExceptionFrame * frame;
	uint32_t * new_sp;

	/* Signal handler being NULL means, that thread ignores signals. No delivery is performed. */
	if (os_threads[thread_id].signal_handler == NULL)
	{
		return;
	}

	if (os_threads[thread_id].state == THREAD_STATE_RUNNING)
	{
		/* Here it means, that PSP shows to the beginning of exception stack frame.
		 * There is nothing above it.
		 */
		frame = (ExceptionFrame *) __get_PSP();
	}
	else
	{
		/* Here it means, that SP shows to the beginning of thread state record.
		 * Thread state record is basically just an exception frame, which has
		 * additional registers placed on top of it.
		 */
		frame = (ExceptionFrame *) (os_threads[thread_id].sp + 8);
		uint32_t * old_sp = os_threads[thread_id].sp;
		new_sp = old_sp - 6;
		for (int q = 0; q < 8; ++q)
		{
			new_sp[q] = old_sp[q];
		}
	}

	/* Create space for 5 values: R0 - R3, PC */
	ExceptionFrame * signal_frame = shim_exception_frame(frame, 6);

#define STACK_BASE	5

	set_exception_argument(signal_frame, STACK_BASE - 1, (uint32_t) signal_frame->lr);

	/* Save R0 - R4 */
	for (int q = 0; q < 4; ++q)
	{
		set_exception_argument(signal_frame, STACK_BASE + q, get_exception_argument(signal_frame, q));
	}
	/* Save PC */
	/* Note that bit 0 is programmatically set to 1. Otherwise CPU will freak out during
	 * return. Exception frame stores PC as verbatim value. If this is used for loading PC
	 * other way than loading from exception frame, then CPU attempts to switch into ARM
	 * mode, which makes Cortex-M sad panda.
	 */
	set_exception_argument(signal_frame, STACK_BASE + 4, (uint32_t) signal_frame->pc | 1);
	/* R0 - arg[0] - signal_mask */
	set_exception_argument(signal_frame, 0, signals);

	/* R1 - arg[1] - sighandler */
	set_exception_argument(signal_frame, 1, (uint32_t) os_threads[thread_id].signal_handler);

	/* PC - os_fire_signal */
	signal_frame->pc = os_fire_signal;

	if (os_threads[thread_id].state == THREAD_STATE_RUNNING)
	{
		__set_PSP((uint32_t *) signal_frame);
	}
	else
	{
		os_threads[thread_id].sp = (uint32_t *) new_sp;
	}
}

int os_kill(uint8_t thread_id, uint8_t signal_id)
{
	ASSERT(thread_id < OS_THREADS);
	if (os_threads[thread_id].state == THREAD_STATE_READY 
			|| os_threads[thread_id].state == THREAD_STATE_RUNNING
			|| os_threads[thread_id].state == THREAD_STATE_STOPPED
			)
	{
		if (signal_id < 32)
		{
			os_threads[thread_id].signals |= 1 << signal_id;
			os_deliver_signal(thread_id, os_threads[thread_id].signals);

			if (os_threads[thread_id].state == THREAD_STATE_STOPPED)
			{
				os_thread_continue(thread_id);
			}
		}
		else
		{
			if (signal_id == SIGSTOP)
			{
				os_thread_stop(thread_id);
			} 
			else if (signal_id == SIGCONT)
			{
				os_thread_continue(thread_id);
			} 
			else if (signal_id == SIGKILL)
			{
				os_thread_kill(thread_id, -SIGCONT);
			}
			else if (signal_id == SIGSEGV)
			{
				os_thread_kill(thread_id, -SIGSEGV);
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
