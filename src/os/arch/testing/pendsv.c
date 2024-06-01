#include <stdint.h>
#include <stdbool.h>
#include <cmrx/os/runtime.h>

bool schedule_context_switch_called = false;

bool schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
    (void) current_task;
    (void) next_task;
    schedule_context_switch_called = true;
	if (os_threads[current_task].state == THREAD_STATE_RUNNING)
	{
		// only mark leaving thread as ready, if it was runnig before
		// if leaving thread was, for example, quit before calling
		// os_sched_yield, then this would return it back to life
		os_threads[current_task].state = THREAD_STATE_READY;
	}
    os_threads[next_task].state = THREAD_STATE_RUNNING;
	return true;
}


