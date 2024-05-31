#include <stdint.h>
#include <stdbool.h>

bool schedule_context_switch(uint32_t current_task, uint32_t next_task)
{
    (void) current_task;
    (void) next_task;
	return false;
}


