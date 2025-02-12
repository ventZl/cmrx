#pragma once

#include <stdint.h>
#include <arch/sysenter.h>

/** @defgroup api_notify Notifications
 * @ingroup api
 * API for synchronization on shared objects.
 *
 * Notifications are a simple mechanism allowing threads to wait for notifications
 * done by other threads. Notifications are matched to waiting threads based on the
 * address of the object. More than one thread can be waiting for any object.
 *
 * If multiple threads are waiting for single object, then waking up is performed
 * based on their priority. The highest-priority thread is going to be woken-up first.
 * @{
 */

/** Notify waiter for object
 *
 * This syscall will send a notification to the highest priority
 * thread waiting for this specific object. Only one thread is
 * notified, the one with highest priority. If no thread is
 * waiting for this specific object, then the call does nothing.
 *
 * @param object object used to determine waiters to be notified
 * @returns E_OK if notification has been sent. E_OUT_OF_NOTIFICATIONS
 * if there is too many different pending notifications and there is
 * nobody waiting for a notification on this particular object.
 */
__SYSCALL int notify_object(const void * object);

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
__SYSCALL int wait_for_object(const void * object, uint32_t timeout);

/** @} */
