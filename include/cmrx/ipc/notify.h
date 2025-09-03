#pragma once

#include <stdint.h>
#include <arch/sysenter.h>
#include <cmrx/sys/notify.h>

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
 * waiting for this specific object, then the notification is
 * recorded and notifying thread returns immediately.
 *
 * @param object object used to determine waiters to be notified
 * @returns E_OK if notification has been sent. E_OUT_OF_NOTIFICATIONS
 * if there is too many different pending notifications and there is
 * nobody waiting for a notification on this particular object.
 */
__SYSCALL int notify_object(const void * object);

/** Notify current waiter for object
 *
 * This syscall performs action similar to @ref notify_object with
 * possibility of changing the call behavior using flags.
 *
 * @param object object used to determine waiters to be notified
 * @param flags optional flags changing call behavior
 * @returns E_OK if notification has been sent. E_OUT_OF_NOTIFICATIONS
 * if there is too many different pending notifications and there is
 * nobody waiting for a notification on this particular object.
 */
__SYSCALL int notify_object2(const void * object, uint32_t flags);

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
 */
__SYSCALL int wait_for_object(const void * object, uint32_t timeout);

/** Wait if value stored at the address is different than one expected.
 *
 * This syscall behaves similarly like @ref wait_for_object with one difference:
 *
 * Thread will be blocked if and only if the value at address pointed to is different
 * than expected value provided. The check-and-lock sequence is atomic even if value is
 * modified concurrently from within another thread.
 *
 * @param object memory area storing the value to be compared with expected value
 * @param value expected value of the memory area
 * @param timeout timeout (in us) after which waiting is aborted and thread
 *                resumes operation. Value of 0 disables timeout and call waits
 *                indefinitely.
 * @param flags flags optional specifiers for behavior of the wait action
 * @returns E_OK or E_OK_NO_WAIT depending on if wait did or didn't occur if operation completed successfully.
 * Error E_ACCESS may be returned if the address provided is not accessible by the thread that made the call.
 * Error E_INVALID_ADDRESS is reported if address is otherwise invalid (out of memory range, etc.).
 * Error E_INVALID is reported if unsupported flag is requested.
 */
__SYSCALL int wait_for_object_value(const uint8_t * object, uint8_t value, uint32_t timeout, uint32_t flags);

/** @} */
