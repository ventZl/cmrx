#include "notify.h"
#include "runtime.h"
#include "sched.h"
#include "txn.h"
#include <cmrx/assert.h>
#include <conf/kernel.h>
#include <kernel/trace.h>

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
    Txn_t txn_id = 0;
    Thread_t thread_id = 0;
    bool awaken = false;
    do {
        txn_id = os_txn_start();
        if (os_threads[thread_id].state == THREAD_STATE_WAITING
            &&  os_threads[thread_id].wait_object == object)
        {
            if (os_txn_commit(txn_id, TXN_READWRITE) == E_OK)
            {
                struct OS_thread_t * notified_thread = &os_threads[thread_id];
                awaken = true;
                notified_thread->state = THREAD_STATE_READY;
                if (notified_thread->wait_callback != NULL)
                {
                    notified_thread->wait_callback(object, thread_id, EVT_USERSPACE_NOTIFICATION);
                    notified_thread->wait_callback = NULL;
                }

                notified_thread->wait_object = NULL;
                os_txn_done();
            } else {
                txn_id = os_txn_start();
                continue;
            }

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

    trace_event(EVENT_NOTIFY_WAIT_INIT, (uint32_t) object);

    return 0;
}


int os_notify_object(const void * object, Event_t event)
{
    Thread_t candidate_thread = 0;
    uint8_t candidate_priority = 0xFF;

    int retval = E_OK;

    Txn_t txn_id = 0;
    do {
        txn_id = os_txn_start();
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
    } while (os_txn_commit(txn_id, TXN_READWRITE) != E_OK);

    // Quirk: only idle thread should have this priority
    if (candidate_priority != 0xFF)
    {
        struct OS_thread_t * notified_thread = &os_threads[candidate_thread];
        notified_thread->state = THREAD_STATE_READY;
        if (notified_thread->wait_callback != NULL)
        {
            trace_event(EVENT_NOTIFY_WAITING, (uint32_t) object);
            notified_thread->wait_callback(object, candidate_thread, event);
            notified_thread->wait_callback = NULL;
        }

        notified_thread->wait_object = NULL;
    }
    else
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
            trace_event(EVENT_NOTIFY_PENDING, (uint32_t) object);
        }
        else
        {
            retval = E_OUT_OF_NOTIFICATIONS;
        }

    }
    os_txn_done();

    if (candidate_priority != 0xFF)
    {
        os_sched_yield();
    }
    return retval;
}

int os_sys_notify_object(const void * object)
{
    return os_notify_object(object, EVT_USERSPACE_NOTIFICATION);
}

/**
* @ingroup os_notify
* @{
*/
/** Callback on receiving a notification.
 * @warning This callback is not executed within the context of the notified
 * thread, rather in the context of notifying thread. The state of notified
 * thread is largely pre-notification at this point.
 *
 * Callback can not assume anything about the future state of the thread
 * post this notification other than the thread will become READY for
 * scheduling.
 */
static void cb_syscall_notify_object(const void * object, Thread_t thread, Event_t event)
{
    (void) object;
    (void) thread;
    (void) event;
    return;
}
/** @} */

int os_sys_wait_for_object(const void * object, uint32_t timeout)
{
    if (timeout != 0)
    {
        // Timeouts are not supported yet.
        // Doing so will most probably require switching the timer
        // subsystem from signals to notifications internally.
        return E_NOTAVAIL;
    }
    return os_wait_for_object(object, NULL);
}

int os_wait_for_object(const void * object, WaitHandler_t callback)
{

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
                    trace_event(EVENT_WAIT_PENDING, (uint32_t) object);
                    pending = true;
                    break;
                }
            }
            if (!pending)
            {
                trace_event(EVENT_WAIT_SUSPEND, (uint32_t) object);
                os_threads[current_thread].state = THREAD_STATE_WAITING;
                os_threads[current_thread].wait_object = object;
                os_threads[current_thread].wait_callback = callback;
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
