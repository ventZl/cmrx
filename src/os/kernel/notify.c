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

struct NotificationObject os_notification_buffer[OS_NOTIFICATION_BUFFER_SIZE];

void os_notify_init()
{
    for (unsigned q = 0; q < OS_NOTIFICATION_BUFFER_SIZE; ++q)
    {
        os_notification_buffer[q].address = (const void *) OS_NOTIFY_INVALID;
        os_notification_buffer[q].pending_notifications = 0;
    }
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

    uint8_t entry_id = 0;
    txn_id = os_txn_start();
    do {
        struct NotificationObject * current_entry = &os_notification_buffer[entry_id];
        if (current_entry->address == object)
        {
            if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK)
            {
                ASSERT(current_entry->pending_notifications > 0);
                current_entry->pending_notifications = 0;
                current_entry->address = (void *) OS_NOTIFY_INVALID;
                os_txn_done();
            }
            else
            {
                txn_id = os_txn_start();
                continue;
            }

            txn_id = os_txn_start();
        }
        entry_id++;
    } while (entry_id < OS_NOTIFICATION_BUFFER_SIZE);

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

static int os_notify_queue_event(const void * object)
{
    struct NotificationObject * first_free_entry = NULL;
    struct NotificationObject * current_object_entry = NULL;

    for (uint8_t entry_id = 0; entry_id < OS_NOTIFICATION_BUFFER_SIZE; ++entry_id)
    {
        struct NotificationObject * current_entry = &os_notification_buffer[entry_id];
        if (current_entry->address == (void *) OS_NOTIFY_INVALID)
        {
            if (first_free_entry == NULL)
            {
                first_free_entry = current_entry;
            }
        }
        if (current_entry->address == object)
        {
            current_object_entry = current_entry;
            break;
        }
    }
    if (current_object_entry == NULL && first_free_entry != NULL)
    {
        first_free_entry->address = object;
        ASSERT(first_free_entry->pending_notifications == 0);
        current_object_entry = first_free_entry;
    }

    if (current_object_entry != NULL)
    {
        current_object_entry->pending_notifications += 1;
    }
    else
    {
        return E_OUT_OF_NOTIFICATIONS;
    }

    return E_OK;
}

Thread_t os_find_notified_thread(const void * object)
{
    Thread_t candidate_thread = 0xFF;
    uint8_t candidate_priority = 0xFF;
    for (int q = 0; q < OS_THREADS; ++q)
    {
        if (os_threads[q].state == THREAD_STATE_WAITING
            &&  os_threads[q].wait_object == object)
        {
            if (os_threads[q].priority < candidate_priority)
            {
                candidate_thread = q;
                candidate_priority = os_threads[q].priority;
            }
        }
    }

    return candidate_thread;
}

int os_notify_object(const void * object, Event_t event, uint32_t flags)
{
    Thread_t candidate_thread = 0;
    int candidate_timer = 0;
    bool perform_thread_switch = false;

    int retval = E_OK;

    Txn_t txn_id = 0;
    do {
        txn_id = os_txn_start();

        candidate_thread = os_find_notified_thread(object);
        candidate_timer = os_find_timer(candidate_thread, TIMER_TIMEOUT);
    } while (os_txn_commit(txn_id, TXN_READWRITE) != E_OK);

    if (candidate_thread < OS_THREADS)
    {
        if (os_notify_thread(candidate_thread, candidate_timer, event) == E_OK)
        {
            if (flags & NOTIFY_PRIORITY_DROP)
            {
                os_threads[os_get_current_thread()].priority = os_threads[os_get_current_thread()].base_priority;
                perform_thread_switch = true;
            }

            if (os_threads[candidate_thread].priority < os_threads[os_get_current_thread()].priority)
            {
                perform_thread_switch = true;
            }

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

int os_sys_wait_for_object_value(const uint8_t * object, uint8_t value, uint32_t timeout, uint32_t flags)
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
        return E_OK_NO_WAIT;
    }

    if ((flags & 0xFF & ~WAIT_FOR_OBJECT_FLAGS_MASK) != 0)
    {
        return E_INVALID;
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
            bool pending = false;
            for (uint8_t entry_id = 0; entry_id < OS_NOTIFICATION_BUFFER_SIZE; ++entry_id)
            {
                struct NotificationObject * current_entry = &os_notification_buffer[entry_id];
                if (current_entry->address == object)
                {
                    ASSERT(current_entry->pending_notifications > 0);
                    const uint32_t remaining_notifications = --current_entry->pending_notifications;
                    if (remaining_notifications == 0)
                    {
                        current_entry->address = (void *) OS_NOTIFY_INVALID;
                    }
                    pending = true;
                    break;
                }
            }
            if (!pending)
            {
                struct OS_thread_t * curr_thread = &os_threads[current_thread];
                if (flags & NOTIFY_PRIORITY_INHERIT_FLAG)
                {
                    struct OS_thread_t * inherit_thread = &os_threads[NOTIFY_PRIORITY_INHERIT_THREAD(flags)];
                    inherit_thread->priority = curr_thread->priority;
                }
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
