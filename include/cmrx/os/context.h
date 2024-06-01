#pragma once

#include <stdbool.h>
#include <cmrx/os/runtime.h>

/** @defgroup os_context Context switching
 *
 * @ingroup os
 *
 * Kernel internals in support context switching
 * 
 * Bulk of the context switching is done inside the platform independent
 * portion of the kernel, but the final heavy lifting always has to be done
 * in a platform specific way. The structures defined here help to establish
 * communication between these two parts of the kernel.
 */

/** @ingroup os_context
 * @{
 */

/** Structure describing upcoming context switch */
struct OS_scheduling_context_t {
    struct OS_thread_t * old_task;
    struct OS_thread_t * new_task;

    struct OS_process_t * old_parent_process;
    struct OS_process_t * new_parent_process;

    struct OS_process_t * old_host_process;
    struct OS_process_t * new_host_process;

    uint32_t * new_stack;
};

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

extern bool ctxt_switch_pending;
extern struct OS_scheduling_context_t cpu_context;
