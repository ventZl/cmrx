#pragma once

/** @defgroup api_notify Synchronization
 *
 * @ingroup api
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
 * @ingroup api_notify
 * @{
 */

#include <arch/sysenter.h>

/** Notify waiter for object
 *
 * This syscall will send a notification to the highest priority
 * thread waiting for this specific object. Only one thread is
 * notified, the one with highest priority. If no thread is
 * waiting for this specific object, then the call does nothing.
 *
 * @param object object used to determine waiters to be notified
 * @returns E_OK. Mostly.
 */
__SYSCALL int irq_enable(uint32_t irq);

/** Wait for a notification on object
 *
 * Calling this syscall will block the thread until any other thread
 * notifies the same object and no other higher-priority thread
 * is waiting for the same object.
 *
 * Optionally the thread can provide a timeout value. If there is no
 * notification on object for given period of time, then waiting is
 * aborted and thread will resume its operation with error return
 * from this call.
 * @param object object used to determine resuming notifications
 * @param timeout timeout timeout (in us) after which waiting is
 *                aborted and thread resumes operation. Value of 0
 *                disables timeout and the call waits indefinitely.
 * @returns E_OK if notification has arrived. E_TIMEOUT if notification
 * did not arrive before the timeout interval has passed.
 * @note Timeouts are not supported as of now. Syscall returns E_NOTAVAIL
 * if timeout value other than 0 is supplied to the call.
 */
__SYSCALL int irq_disable(uint32_t irq);

/* @} */
