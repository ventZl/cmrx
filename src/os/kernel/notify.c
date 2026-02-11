#include "notify.h"
#include "runtime.h"
#include "sched.h"
#include "arch/sched.h"
#include "txn.h"
#include "timer.h"
#include "access.h"
#include <cmrx/assert.h>
#include <conf/kernel.h>
#include <cmrx/sys/notify.h>
#include "algo.h"

static struct NotificationObject os_notification_buffer[OS_NOTIFICATION_BUFFER_SIZE];
unsigned os_notification_buffer_size;

static Thread_t os_notification_waiters[OS_THREADS];
unsigned os_notification_waiters_size;

static unsigned os_notify_find_queue_entry(const void * object)
{
    return HASH_SEARCH(os_notification_buffer, address, object, OS_NOTIFICATION_BUFFER_SIZE);
}

static int os_notify_queue_event(const void * object)
{
    unsigned offs = os_notify_find_queue_entry(object);

    if (os_notification_buffer_size == OS_NOTIFICATION_BUFFER_SIZE)
    {
        return E_OUT_OF_NOTIFICATIONS;
    }

    struct NotificationObject * notification_entry = &os_notification_buffer[offs];
    if (notification_entry->address == (const void *) HASH_EMPTY)
    {
        ASSERT(notification_entry->pending_notifications == 0);
        notification_entry->address = object;
        os_notification_buffer_size++;
    }

    notification_entry->pending_notifications += 1;

    return E_OK;
}

static bool os_notify_dequeue_event(const void * object)
{
    unsigned offs = os_notify_find_queue_entry(object);
    struct NotificationObject * current_entry = &os_notification_buffer[offs];

    if (current_entry->address != object)
    {
        return false;
    }
    ASSERT(current_entry->pending_notifications > 0);
    const uint32_t remaining_notifications = --current_entry->pending_notifications;
    if (remaining_notifications == 0)
    {
        current_entry->address = (void *) HASH_EMPTY;
        os_notification_buffer_size--;
    }

    return true;
}

void os_notify_init(void)
{
    for (unsigned q = 0; q < OS_NOTIFICATION_BUFFER_SIZE; ++q)
    {
        os_notification_buffer[q].address = (const void *) HASH_EMPTY;
        os_notification_buffer[q].pending_notifications = 0;
    }

    os_notification_buffer_size = 0;
    os_notification_waiters_size = 0;
}

int os_initialize_waitable_object(const void* object)
{
    Txn_t txn_id = os_txn_start();
    Thread_t thread_id = 0;
    bool awaken = false;
    do {
        if (os_threads[thread_id].state == THREAD_STATE_WAITING
            &&  os_threads[thread_id].wait_object == object)
        {
            int sleeper_id = os_find_timer(thread_id, TIMER_TIMEOUT);
            if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK)
            {
                if (os_notify_thread(thread_id, sleeper_id, EVT_DEFAULT) == E_YIELD)
                {
                    awaken = true;
                }

                os_txn_done();
            }

            // Success or not, new transaction has to be created
            // as the old one has been already used
            txn_id = os_txn_start();
        }
        thread_id++;
    } while (thread_id < OS_THREADS);

    txn_id = os_txn_start();
    do {
        unsigned offs = os_notify_find_queue_entry(object);
        if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK)
        {
            struct NotificationObject * current_entry = &os_notification_buffer[offs];

            if (current_entry->address == object)
            {
                ASSERT(current_entry->pending_notifications > 0);
                current_entry->address = (void *) HASH_EMPTY;
                current_entry->pending_notifications = 0;
                os_notification_buffer_size--;
            }

            os_txn_done();
            break;
        }
        else
        {
            txn_id = os_txn_start();
        }
    } while (true);

    if (awaken)
    {
        os_sched_yield();
    }

    return E_OK;
}

int os_notify_thread(Thread_t thread_id, int candidate_timer, Event_t event)
{
    struct OS_thread_t * notified_thread = &os_threads[thread_id];
    int retval = E_OK;

    if (notified_thread->wait_object == NULL)
    {
        return E_NOTAVAIL;
    }

    if (notified_thread->wait_callback != NULL)
    {
        notified_thread->wait_callback(notified_thread->wait_object, thread_id, candidate_timer, event);
        notified_thread->wait_callback = NULL;
    }

    notified_thread->wait_object = NULL;

    return retval;
}

unsigned os_find_notified_thread_waiter(const void * object)
{
    unsigned candidate_waiter = 0xFF;
    uint8_t candidate_priority = 0xFF;
    for (unsigned int waiter = 0; waiter < os_notification_waiters_size; ++waiter)
    {
        Thread_t waiting_thread = os_notification_waiters[waiter];
        ASSERT(os_threads[waiting_thread].state == THREAD_STATE_WAITING);
        if (os_threads[waiting_thread].wait_object == object)
        {
            if (os_threads[waiting_thread].priority < candidate_priority)
            {
                candidate_waiter = waiter;
                candidate_priority = os_threads[waiting_thread].priority;
            }
        }
    }

    return candidate_waiter;
}

int os_notify_object(const void * object, Event_t event, uint32_t flags)
{
    unsigned candidate_waiter = 0;
    int candidate_timer = 0;
    bool perform_thread_switch = false;

    int retval = E_OK;

    Txn_t txn_id = 0;
    do {
        txn_id = os_txn_start();

        candidate_waiter = os_find_notified_thread_waiter(object);
        candidate_timer = os_find_timer(os_notification_waiters[candidate_waiter], TIMER_TIMEOUT);
    } while (os_txn_commit(txn_id, TXN_READWRITE) != E_OK);

    if (candidate_waiter < OS_THREADS)
    {
        if (os_notify_thread(os_notification_waiters[candidate_waiter], candidate_timer, event) == E_OK)
        {
            struct OS_thread_t * current_thread = &os_threads[os_get_current_thread()];
            if (flags & NOTIFY_PRIORITY_DROP)
            {
                current_thread->priority = current_thread->base_priority;
                perform_thread_switch = true;
            }

            if (os_threads[os_notification_waiters[candidate_waiter]].priority < current_thread->priority)
            {
                perform_thread_switch = true;
            }

            ARRAY_DELETE(os_notification_waiters, candidate_waiter, os_notification_waiters_size);
        }
    }
    else
    {
        if (flags & NOTIFY_QUEUE_NOTIFICATION)
        {
            retval = os_notify_queue_event(object);
        }

    }
    os_txn_done();

    if (perform_thread_switch)
    {
        os_sched_yield();
    }
    return retval;
}

int os_sys_notify_object(const void * object)
{
    return os_notify_object(object, EVT_DEFAULT, NOTIFY_QUEUE_NOTIFICATION);
}

int os_sys_notify_object2(const void * object, uint32_t flags)
{
    if ((flags & 0xFF & ~NOTIFY_OBJECT_FLAGS_MASK) != 0)
    {
        return E_INVALID;
    }

    return os_notify_object(object, EVT_DEFAULT, flags);
}

/**
* @ingroup os_notify
* @{
*/
/** Callback implementing reception of a userspace notification
 * @warning This callback is not executed within the context of the notified
 * thread, rather in the context of notifying thread. The state of notified
 * thread is largely pre-notification at this point.
 *
 * This callback will manipulate thread state when notification is received.
 * Handling is different when notification is received via @ref notify_object
 * and wait reaching timeout value.
 */
void cb_syscall_notify_object(const void * object, Thread_t thread, int sleeper, Event_t event)
{
    (void) object;

    struct OS_thread_t * notified_thread = &os_threads[thread];

    // Notification may have arrived before the thread has been scheduled out
    // and from the scheduler's point of view it is still running.
    // Keep its state as "running" so it won't be swapped out. It is apparently
    // the thread with the highest priority.
    if (core[coreid()].thread_current != thread)
    {
        notified_thread->state = THREAD_STATE_READY;
    }
    else
    {
        notified_thread->state = THREAD_STATE_RUNNING;
    }

    switch (event) {
        case EVT_DEFAULT:
            os_cancel_sleeper(sleeper);
            break;

        case EVT_TIMEOUT:
            os_set_syscall_return_value(thread, E_TIMEOUT);
            break;
    }
}
/** @} */

int os_sys_wait_for_object(const void * object, uint32_t timeout)
{
    if (timeout != 0)
    {
        os_set_timed_event(timeout, TIMER_TIMEOUT);
    }
    return os_wait_for_object(object, cb_syscall_notify_object, 0);
}

int os_sys_wait_for_object_value(uint8_t * object, uint8_t value, uint32_t timeout, uint32_t flags)
{
    if (object == NULL)
    {
        return E_INVALID_ADDRESS;
    }

    if (!os_thread_check_access(os_get_current_thread(), (uint32_t *) object, ACCESS_READ_WRITE))
    {
        return E_ACCESS;
    }

    if (*object == value)
    {
        if (flags & NOTIFY_VALUE_INCREMENT)
        {
            os_txn_start_commit();
            // @TODO: This needs to be atomic
            *object+=1;
            os_txn_done();
        }
        return E_OK_NO_WAIT;
    }

    if ((flags & 0xFF & ~WAIT_FOR_OBJECT_FLAGS_MASK) != 0)
    {
        return E_INVALID;
    }

    if (timeout != 0)
    {
        os_set_timed_event(timeout, TIMER_TIMEOUT);
    }
    return os_wait_for_object(object, cb_syscall_notify_object, flags);
}

int os_wait_for_object(const void * object, WaitHandler_t callback, uint32_t flags)
{
    if (callback == NULL)
    {
        return E_INVALID;
    }

    Txn_t txn_id = os_txn_start();

    Thread_t current_thread = os_get_current_thread();
    enum ThreadState thread_state = os_threads[current_thread].state;
    while (thread_state == THREAD_STATE_READY || thread_state == THREAD_STATE_RUNNING)
    {
        if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK) {
            bool pending = os_notify_dequeue_event(object);
            if (!pending)
            {
                struct OS_thread_t * curr_thread = &os_threads[current_thread];
                if (flags & NOTIFY_PRIORITY_INHERIT_FLAG)
                {
                    uint8_t inherit_id = NOTIFY_PRIORITY_INHERIT_THREAD(flags);
                    if (inherit_id < OS_THREADS
                        && (os_threads[inherit_id].state == THREAD_STATE_READY
                            || os_threads[inherit_id].state == THREAD_STATE_WAITING
                            || os_threads[inherit_id].state == THREAD_STATE_RUNNING
                            || os_threads[inherit_id].state == THREAD_STATE_STOPPED
                            || os_threads[inherit_id].state == THREAD_STATE_MIGRATING)
                        )
                    {
                        struct OS_thread_t * inherit_thread = &os_threads[inherit_id];
                        inherit_thread->priority = curr_thread->priority;
                    }
                    else
                    {
                        os_txn_done();
                        return E_INVALID;
                    }
                }

                os_notification_waiters[os_notification_waiters_size++] = current_thread;
                curr_thread->state = THREAD_STATE_WAITING;
                curr_thread->wait_object = object;
                curr_thread->wait_callback = callback;
            }
            os_txn_done();
            if (!pending)
            {
                os_sched_yield();
            }
            return E_OK;
        } else {
            txn_id = os_txn_start();
            thread_state = os_threads[current_thread].state;
        }
    }
    return E_BUSY;
}

bool os_is_object_waited_for(void * object)
{
    Txn_t txn_id = 0;
    bool waiting = false;
    do {
         txn_id = os_txn_start();

         for (int q = 0; q < OS_THREADS; ++q)
         {
             if (os_threads[q].state == THREAD_STATE_WAITING
                 && os_threads[q].wait_object == object)
             {
                 waiting = true;
                 break;
             }
         }
    } while (os_txn_commit(txn_id, TXN_READONLY) != E_OK);

    return waiting;
}
