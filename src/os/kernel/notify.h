/** @defgroup os_notify Notifications
 * Kernel internals supporting cross-process notifications.
 * Notifications are concept that can be used to notify other threads implicitly
 * without knowing their identity. The identity of notified  thread is established
 * based on the match of the object notified and waited for.
 *
 * Thread which is waiting for notification can install a kernel-space callback that
 * gets called before the thread is resumed.
 *
 * @ingroup os
 * @{
 */
#pragma once

#include "runtime.h"

/** List of possible notification types.
 */
enum EventTypes {
	/// Plain userspace notification done via notify_object syscall
	EVT_USERSPACE_NOTIFICATION = 0,

    /// Emitted in case thread
    EVT_THREAD_DONE,
	_EVT_COUNT
};

/** Resume one thread that are waiting for this object.
 * This function will find one single thread which is waiting for this particular
 * object. If more than one thread is waiting for this object, then the thread with
 * the highest priority will be resumed.
 * @param object address of the object that is notified
 * @param event event that is signalled to the object
 * @returns E_OK if any thread was waiting for this object and was woken up
 * @note The object address is mostly meaningless. It allows threads to synchronize
 * on any objects both externally from the userspace and internally in the kernel.
 */
int os_notify_object(const void * object, Event_t event);

/** Wait for object.
 * This function will block thread until some other thread notifies the object.
 * Threads are being woken up based on their priorities. Highest priority first.
 * @param object address of the object that is being waited for
 * @param callback address of the callback function that handles wakeup
 * @returns E_OK if thread is capable of waiting, E_BUSY if thread is already
 * waiting for something.
 */
int os_wait_for_object(const void * object, WaitHandler_t callback);

/** Implementation of notify_object syscall.
 * This is a wrapper around os_notify_object system call. It does some additional
 * checking on arguments.
 * See @ref notify_object and @red os_notify_object for more info.
 */
int os_sys_notify_object(const void * object);

/** Implementation of wait_for_object syscall.
 * This is a wrapper around os_wait_for_object system call. It does some additional
 * checking on arguments.
 * See @ref wait_for_object and @red os_wait_for_object for more info.
 */
int os_sys_wait_for_object(const void * object, uint32_t timeout);

/** @} */
