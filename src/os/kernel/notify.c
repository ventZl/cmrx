#include <cmrx/os/notify.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/txn.h>

int os_notify_object(const void * object, Event_t event)
{
    Thread_t candidate_thread = 0;
    uint8_t candidate_priority = 0xFF;

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
            notified_thread->wait_callback(object, candidate_thread, event);
            notified_thread->wait_callback = NULL;
        }

        notified_thread->wait_object = NULL;
    }
    os_txn_done();
    if (candidate_priority != 0xFF)
    {
        os_sched_yield();
    }
    return E_OK;
}

int os_sys_notify_object(const void * object)
{
    return os_notify_object(object, EVT_USERSPACE_NOTIFICATION);
}

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
            os_threads[current_thread].state = THREAD_STATE_WAITING;
            os_threads[current_thread].wait_object = object;
            os_threads[current_thread].wait_callback = callback;
            os_txn_done();
            os_sched_yield();
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
