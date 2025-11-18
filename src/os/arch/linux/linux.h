#pragma once

#include <kernel/runtime.h>

/** @ingroup arch_linux_impl
 * @{
 */

/** Internal thread startup data structure
 *
 * Internal structure to communicate CMRX thread
 * details to the routine which initiates Linux
 * thread that hosts the CMRX thread.
 */
struct thread_startup_t {
    int thread_id;              ///< CMRX thread ID
    entrypoint_t * entry_point; ///< thread entry function as CMRX userspace sees it
    void * entry_arg;           ///< argument to the entry function
};


/** Initiate thread switch sequence if it was requested.
 * This function will initiate the thread switch sequence.
 * Sequence is actually performed by other code, which runs
 * in different thread. This function just sends a signal
 * to run it.
 *
 * It is safe to call this function from any thread.
 */
void trigger_pendsv_if_needed();

/**
 * @}
 */
