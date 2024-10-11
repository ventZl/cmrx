#pragma once

/** @addtogroup arch_arch
 *
 * CMRX offers primitives to perform unaddressed synchronization
 * unlike @ref kill() call, which allows thread to send a signal
 * to another thread, API in this group uses memory location as
 * a designator, rather than specific thread.
 *
 * Using this API, a programmer can write code that notifies
 * unknown recipient. All that is needed for notifier and waiter
 * for this mechanism to work is to agree upon memory location used as
 * notification object.
 *
 * Any object can be used as the object itself is not examined or
 * used during the notification. It is just a convenient way on
 * determining the notification ID.
 *
 * It allows multiple object to wait for single object and thus
 * can serve as a building block for more advanced primitives such
 * as semaphores or queues.
 *
 * @ingroup arch_arch
 * @{
 */

/** Request Inter-processor synchronization.
 * This is minimalistic implementation for inter-processor synchronization 
 * request for Cortex-M processors. This implementation does not assume
 * presence of any inter-processor interrupt mechanism. It assumes that
 * each core checks for inter-processor sync every now and then.
 * This function will block until all online cores call os_ipi_sync_probe().
 * By doing so all cores remain synchronized and it is guarranteed they will
 * do nothing. Then this function returns.
 */
void os_ipi_sync_request();

/** Release cores waiting in inter-processor synchronization loop.
 * This function will release all cores which are waiting in os_ipi_sync_probe()
 * function waiting for core which requested synchronization to release it.
 * Failure to call this function after os_ipi_sync_request() was called will 
 * result in permanent halt of all other system cores.
 */
void os_ipi_sync_release();

/** Check if inter-processor synchronization is requested.
 * Each core should call this function from time to time. If any other core
 * calls for inter-processor synchronization, then core calling this function will
 * be blocked in the call until the core which requested the synchronization doesn't 
 * release it.
 */
void os_ipi_sync_probe();

