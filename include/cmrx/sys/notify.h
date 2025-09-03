#pragma once

/** @defgroup notify_flags
 * @ingroup api_notify
 * Flags that further specify the behavior of calls in notification group.
 * @{
 */

#define NOTIFY_PRIORITY_INHERIT_FLAG        (1 << 0)
#define NOTIFY_PRIORITY_DROP_FLAG           (1 << 1)
#define NOTIFY_QUEUE_NOTIFICATION_FLAG      (1 << 2)

/** Inherit scheduler priority to specific thread if caller is going to be suspended.
 * This flag tells the kernel to inherit scheduler priority of the current thread to
 * specific thread ID. This only happens if calling thread is going to be suspended
 * waiting for notification. If specific thread already has priority higher than that
 * of the calling thread then priority change does not happen.
 * @param thread_id ID of the thread to inherit the priority
 */
#define NOTIFY_PRIORITY_INHERIT(thread_id) (((thread_id & 0xFF) << 8) | NOTIFY_PRIORITY_INHERIT_FLAG)

/** Drop inherited priority after notification has been sent.
 * Calling thread will drop elevated priority after notifications have been sent and return
 * to its default priority.
 */
#define NOTIFY_PRIORITY_DROP                (NOTIFY_PRIORITY_DROP_FLAG)

/** If there is no object waiting for notification then queue notification for later.
 * Enabling this flag will cause that any notification that won't immediately wake up
 * any thread will be queued into pending notifications list.
 */
#define NOTIFY_QUEUE_NOTIFICATION           (NOTIFY_QUEUE_NOTIFICATION_FLAG)
/** Statement that there are no flags passed to the call.
 */
#define NOTIFY_NO_FLAGS                     0

#define NOTIFY_PRIORITY_INHERIT_THREAD(flags)   ((flags >> 8) && 0xFF)

#define WAIT_FOR_OBJECT_FLAGS_MASK          (NOTIFY_PRIORITY_INHERIT_FLAG)
#define NOTIFY_OBJECT_FLAGS_MASK            (NOTIFY_PRIORITY_DROP_FLAG | NOTIFY_QUEUE_NOTIFICATION)

/** @} */
