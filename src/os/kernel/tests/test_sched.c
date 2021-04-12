#include "../sched.c"
#include <ctest.h>

bool schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
	return false;
}

bool os_schedule_timer(unsigned * delay)
{
	return false;
}

void os_run_timer(uint32_t microtime)
{
}

int thread_exit(int status)
{
}

void os_timer_init()
{
}

int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint32_t flags)
{
}

CTEST(sched, yield) {
}
