#include <kernel/notify.h>
#include <ctest.h>
#include <arch/corelocal.h>
extern struct OS_core_state_t core[OS_NUM_CORES];

static bool notify_called = false;
static const void * notify_object = NULL;
static Thread_t notify_thread = 0;
static Event_t notify_event = 0;


CTEST_DATA(os_notify_wait_object) {
};

CTEST_SETUP(os_notify_wait_object) {
    for (unsigned q = 0; q < OS_THREADS; ++q)
    {
        os_threads[q].wait_object = NULL;
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

    notify_called = false;
    notify_object = NULL;
    notify_thread = 0;
    notify_event = 0;
}

static void * const object_magic = (void *) 0x12345678;
static void * const another_object_magic = (void *) 0x87654321;

CTEST2(os_notify_wait_object, os_wait_object) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[0].wait_callback, (long) NULL);

    ASSERT_EQUAL(core[0].thread_current, 3);
}

CTEST2(os_notify_wait_object, os_wait_object_stopped) {
    core[0].thread_current = 1;

    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_BUSY);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_STOPPED);
    ASSERT_EQUAL((long) os_threads[1].wait_object, (long) NULL);
    ASSERT_EQUAL((long) os_threads[1].wait_callback, (long) NULL);

    ASSERT_EQUAL(core[0].thread_current, 1);
}

CTEST2(os_notify_wait_object, os_wait_object_multiple) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[0].wait_callback, (long) NULL);

    core[0].thread_current = 3;

    rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[3].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[3].wait_object, (long) object_magic);
    ASSERT_EQUAL((long) os_threads[3].wait_callback, (long) NULL);
}

static void cb_object_notify(const void * object, Thread_t thread, Event_t event)
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
    int rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
}

CTEST2(os_notify_wait_object, os_notify_no_callback) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
}

CTEST2(os_notify_wait_object, os_notify_higher_priority) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    core[0].thread_current = 4;

    rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    // This has to be reset to value consistent with os_threads table
    // as notify will call os_sched_yield
    core[0].thread_current = 3;

    rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[4].wait_object, (long) object_magic);
}

CTEST2(os_notify_wait_object, os_notify_multiple) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    core[0].thread_current = 4;

    rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    // This has to be reset to value consistent with os_threads table
    // as notify will call os_sched_yield
    core[0].thread_current = 3;

    rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[4].wait_object, (long) object_magic);

    rv = os_notify_object(object_magic, 42);

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

    rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
    ASSERT_EQUAL(notify_called, true);
    ASSERT_EQUAL((long) notify_object, (long) object_magic);
    ASSERT_EQUAL(notify_event, 42);
}

CTEST2(os_notify_wait_object, os_notify_different_object) {
    int rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_notify_object(another_object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(notify_called, false);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
}

CTEST2(os_notify_wait_object, os_notify_missed) {
    // Notification, that aims at object nobody is waiting for right now
    int rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_wait_for_object(object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);
}

CTEST2(os_notify_wait_object, os_notify_missed_different_object) {
    // Notification, that aims at object nobody is waiting for right now
    int rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_wait_for_object(another_object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) another_object_magic);
}

CTEST2(os_notify_wait_object, os_notify_missed_acts_once) {
    // Notification, that aims at object nobody is waiting for right now
    int rv = os_notify_object(object_magic, 42);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_wait_for_object(object_magic, cb_object_notify);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) NULL);

    rv = os_wait_for_object(object_magic, NULL);

    ASSERT_EQUAL(rv, E_OK);

    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_WAITING);
    ASSERT_EQUAL((long) os_threads[0].wait_object, (long) object_magic);
}
