/** @ingroup os_kernel
 * @{
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Schedule context switch on next suitable moment.
 *
 * This function will tell scheduler, that we want to switch running tasks.
 * Switch itself will be performed on next suitable moment by asynchronous
 * routine. This mechanism is used to avoid context switch in the middle of
 * interrupt routine, or otherwise complicated situation.
 *
 * @param current_task ID of thread which is currently being executed
 * @param next_task ID of thread which should start being executed
 * @returns true if context switch will happen, false otherwise
 */
bool schedule_context_switch(uint32_t current_task, uint32_t next_task);

/** @} */
