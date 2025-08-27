#include <kernel/notify.h>
#include <kernel/timer.h>
#include <kernel/sched.h>
#include <ctest.h>

#include "test_traits.h"

CTEST_DATA(os_notify_futex) {
};

CTEST_SETUP(os_notify_futex) {
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

/*
    notify_called = false;
    notify_object = NULL;
    notify_thread = 0;
    notify_event = 0;*/

/*    schedule_context_switch_perform_switch = true;
    schedule_context_switch_called = false;

    updated_syscall_return_thread = 0xFF;
    updated_syscall_return_value = 0;
    updated_syscall_return_called = false;
    */
}

uint8_t futex = 0;

#define DEFAULT_CALLBACK    cb_syscall_notify_object

CTEST2(os_notify_futex, os_wait_object_same_value) {
    int rv = os_sys_wait_for_object_value(&futex, 0, 0, 0);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_RUNNING);
}

CTEST2(os_notify_futex, os_wait_object_different_value) {
    int rv = os_sys_wait_for_object_value(&futex, 1, 0, 0);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
}


CTEST2(os_notify_futex, os_notify_none) {
    int rv = os_notify_object(&futex, 42, false);

    ASSERT_EQUAL(rv, E_OK);
}
