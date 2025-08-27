#include <kernel/notify.h>
#include <kernel/sched.h>
#include <kernel/timer.h>
#include <ctest.h>
#include <arch/corelocal.h>
extern struct OS_core_state_t core[OS_NUM_CORES];
extern bool schedule_context_switch_perform_switch;
extern bool schedule_context_switch_called;

extern Thread_t updated_syscall_return_thread;
extern uint32_t updated_syscall_return_value;
extern bool updated_syscall_return_called;

static bool notify_called = false;
static const void * notify_object = NULL;
static Thread_t notify_thread = 0;
static Event_t notify_event = 0;
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];

extern void cb_syscall_notify_object(const void * object, Thread_t thread, int sleeper_id, Event_t event);

#define DEFAULT_CALLBACK    cb_syscall_notify_object

CTEST_DATA(os_notify_wait_object) {
};

CTEST_SETUP(os_notify_wait_object) {
    for (unsigned q = 0; q < OS_THREADS; ++q)
    {
        os_threads[q].wait_object = NULL;
        os_threads[q].wait_callback = NULL;
    }

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[0].priority = 32;

    os_threads[1].state = THREAD_STATE_STOPPED;
    os_threads[1].priority = 24;

    // state is THREAD_STATE_EMPTY;
    os_threads[2].priority = 1;

    os_threads[3].state = THREAD_STATE_FINISHED;
    os_threads[3].priority = 128;

    os_threads[3].state = THREAD_STATE_READY;
    os_threads[3].priority = 33;

    os_threads[4].state = THREAD_STATE_READY;
    os_threads[4].priority = 48;

    core[0].thread_current = 0;

    os_notify_init();
    os_timer_init();

    notify_called = false;
    notify_object = NULL;
    notify_thread = 0;
    notify_event = 0;

    schedule_context_switch_perform_switch = true;
    schedule_context_switch_called = false;

    updated_syscall_return_thread = 0xFF;
    updated_syscall_return_value = 0;
    updated_syscall_return_called = false;
}

static void * const object_magic = (void *) 0x12345678;
static void * const another_object_magic = (void *) 0x87654321;

CTEST2(os_notify_wait_object, os_wait_object) {
    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[0].wait_callback, (long) DEFAULT_CALLBACK);

    ASSERT_EQUAL(core[0].thread_current, 3);
}

CTEST2(os_notify_wait_object, os_wait_object_stopped) {
    core[0].thread_current = 1;

    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_BUSY);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_STOPPED);
    ASSERT_EQUAL((long) os_threads[1].wait_object, (long) NULL);
    ASSERT_EQUAL((long) os_threads[1].wait_callback, (long) NULL);

    ASSERT_EQUAL(core[0].thread_current, 1);
}

CTEST2(os_notify_wait_object, os_wait_object_multiple) {
    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[0].wait_callback, (long) DEFAULT_CALLBACK);

    core[0].thread_current = 3;

    rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[3].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[3].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[3].wait_callback, (long) DEFAULT_CALLBACK);
}

static void cb_object_notify(const void * object, Thread_t thread, int sleeper_id, Event_t event)
{
    notify_called = true;
    notify_object = object;
    notify_thread = thread;
    notify_event = event;
}

CTEST2(os_notify_wait_object, os_wait_object_callback) {
    int rv = os_wait_for_object(object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[0].wait_callback, (long) cb_object_notify);
}

CTEST2(os_notify_wait_object, os_notify_none) {
    int rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
}

CTEST2(os_notify_wait_object, os_notify_no_callback) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_INVALID);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL((long) os_threads[0].wait_callback, (long) NULL);
}

CTEST2(os_notify_wait_object, os_notify_higher_priority) {
    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    core[0].thread_current = 4;

    rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    // This has to be reset to value consistent with os_threads table
    // as notify will call os_sched_yield
    core[0].thread_current = 3;

    rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[4].wait_object, (long) object_magic);
}

CTEST2(os_notify_wait_object, os_notify_multiple) {
    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    core[0].thread_current = 4;

    rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    // This has to be reset to value consistent with os_threads table
    // as notify will call os_sched_yield
    core[0].thread_current = 3;

    rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[4].wait_object, (long) object_magic);

    rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_READY);
    ASSERT_EQUAL((long) os_threads[4].wait_object, (long) NULL);
}

CTEST2(os_notify_wait_object, os_notify_callback) {
    int rv = os_wait_for_object(object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(notify_called, true);
    ASSERT_EQUAL((long) notify_object, (long) object_magic);
    ASSERT_EQUAL(notify_event, 42);
}

CTEST2(os_notify_wait_object, os_notify_different_object) {
    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_notify_object(another_object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
}

CTEST2(os_notify_wait_object, os_notify_missed) {
    // Notification, that aims at object nobody is waiting for right now
    int rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_wait_for_object(object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
}

CTEST2(os_notify_wait_object, os_notify_missed_different_object) {
    // Notification, that aims at object nobody is waiting for right now
    int rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_wait_for_object(another_object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) another_object_magic);
}

CTEST2(os_notify_wait_object, os_notify_missed_acts_once) {
    // Notification, that aims at object nobody is waiting for right now
    int rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_wait_for_object(object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);

    rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
}

/* This test tests for a situation, that a thread calls wait_for_object()
 * without pending notification. Thus it gets suspended. Before the thread
 * switch can actually occurr, ISR comes which calls notify_object() for
 * the object that caused this, still current, thread to suspend. The expected
 * outcome of this situation is that the thread will be set into running
 * state as it wasn't scheduled out yet.
 */
CTEST2(os_notify_wait_object, wait_notify_same_thread_running) {
    // We don't want PendSV to actually perform thread switch
    schedule_context_switch_perform_switch = false;

    // Here thread will be suspended but thread switch won't happen
    int rv = os_wait_for_object(object_magic, DEFAULT_CALLBACK);

    ASSERT_EQUAL(rv, E_OK);

    // Thread switch did not happen as we simulate notify_object being called from ISR
    // that executed before PendSV had chance to run.

    schedule_context_switch_called = false;

    // This notification should make the thread running again without
    // actually executing thread switch
    rv = os_notify_object(object_magic, 42, true);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(schedule_context_switch_called, false);
    ASSERT_EQUAL(os_threads[core[coreid()].thread_current].state, THREAD_STATE_RUNNING);
}

CTEST2(os_notify_wait_object, wait_timeout_accepted) {
   int rv = os_sys_wait_for_object(object_magic, 4000);

   ASSERT_EQUAL(rv, E_OK);
}

CTEST2(os_notify_wait_object, sys_wait_timeout_timed_out) {
    int rv = os_sys_wait_for_object(object_magic, 4000);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);

    uint32_t current_time = os_get_micro_time();

    os_run_timer(current_time + 3990);
    os_sched_yield();

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);

    os_run_timer(current_time + 4000);
    os_sched_yield();

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(updated_syscall_return_called, true);
    ASSERT_EQUAL(updated_syscall_return_thread, 0);
    ASSERT_EQUAL(updated_syscall_return_value, E_TIMEOUT);

}

CTEST2(os_notify_wait_object, sys_wait_timeout_notification) {
    int rv = os_sys_wait_for_object(object_magic, 4000);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);

    uint32_t current_time = os_get_micro_time();

    os_run_timer(current_time + 3990);
    os_sched_yield();

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);

    os_sys_notify_object(object_magic);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(updated_syscall_return_called, false);

}
