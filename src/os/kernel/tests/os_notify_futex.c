#include <kernel/notify.h>
#include <kernel/timer.h>
#include <kernel/sched.h>
#include <cmrx/sys/notify.h>
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

extern void cb_syscall_notify_object(const void * object, Thread_t thread, int sleeper_id, Event_t event);

CTEST2(os_notify_futex, os_wait_object_same_value) {
    int rv = os_sys_wait_for_object_value(&futex, 0, 0, 0);

    ASSERT_EQUAL(rv, E_OK_NO_WAIT);
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

// Additional wait_for_object_value tests
CTEST2(os_notify_futex, os_wait_object_value_with_timeout_no_notification) {
    futex = 1;
    int rv = os_sys_wait_for_object_value(&futex, 0, 1000, 0);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
}

CTEST2(os_notify_futex, os_wait_object_value_zero_timeout) {
    futex = 1;
    int rv = os_sys_wait_for_object_value(&futex, 0, 0, 0);

    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
}

CTEST2(os_notify_futex, os_wait_object_value_null_pointer) {
    int rv = os_sys_wait_for_object_value(NULL, 0, 0, 0);

    ASSERT_EQUAL(rv, E_INVALID_ADDRESS);
}

uint8_t futex_second = 5;

CTEST2(os_notify_futex, os_wait_object_value_different_objects) {
    futex = 1;
    futex_second = 2;
    
    int rv1 = os_sys_wait_for_object_value(&futex, 0, 0, 0);
    ASSERT_EQUAL(rv1, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
    
    // Reset for second test - this would normally be done by test framework
    // but we're testing different behavior
    int rv2 = os_sys_wait_for_object_value(&futex_second, 1, 0, 0);
    ASSERT_EQUAL(rv2, E_OK);
}

CTEST2(os_notify_futex, os_wait_object_value_boundary_values) {
    futex = 0;
    int rv1 = os_sys_wait_for_object_value(&futex, 0, 0, 0);
    ASSERT_EQUAL(rv1, E_OK_NO_WAIT);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_RUNNING);
    
    futex = 255;
    int rv2 = os_sys_wait_for_object_value(&futex, 255, 0, 0);
    ASSERT_EQUAL(rv2, E_OK_NO_WAIT);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_RUNNING);
    
    int rv3 = os_sys_wait_for_object_value(&futex, 0, 0, 0);
    ASSERT_EQUAL(rv3, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
}

// notify_immediate tests
CTEST2(os_notify_futex, os_notify_immediate_no_waiters) {
    int rv = os_sys_notify_object2(&futex, NOTIFY_PRIORITY_DROP);

    ASSERT_EQUAL(rv, E_OK);
}

CTEST2(os_notify_futex, os_notify_immediate_vs_regular_no_waiters) {
    // Test that immediate notify doesn't queue notifications when no waiters
    int rv1 = os_sys_notify_object2(&futex, NOTIFY_PRIORITY_DROP);
    ASSERT_EQUAL(rv1, E_OK);
    
    // Now wait - should block since immediate notify didn't queue
    futex = 1;
    int rv2 = os_sys_wait_for_object_value(&futex, 0, 0, 0);
    ASSERT_EQUAL(rv2, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
}

CTEST2(os_notify_futex, os_notify_regular_vs_immediate_queuing) {
    // Test that regular notify queues notifications when no waiters
    int rv1 = os_sys_notify_object(&futex);
    ASSERT_EQUAL(rv1, E_OK);
    
    // Now wait - should NOT block since regular notify queued the notification
    futex = 1;  
    int rv2 = os_sys_wait_for_object_value(&futex, 0, 0, 0);
    ASSERT_EQUAL(rv2, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_RUNNING);
    
    // But the wait should consume the queued notification and succeed immediately
}

CTEST2(os_notify_futex, os_notify_immediate_with_waiter) {
    // Set up a waiting thread (thread 1)
    os_threads[1].state = THREAD_STATE_WAITING;
    os_threads[1].wait_object = &futex;
    os_threads[1].wait_callback = cb_syscall_notify_object;
    
    int rv = os_sys_notify_object2(&futex, NOTIFY_PRIORITY_DROP);
    
    ASSERT_EQUAL(rv, E_OK);
    // The waiting thread should be notified
    ASSERT_NULL(os_threads[1].wait_object);
    ASSERT_NULL(os_threads[1].wait_callback);
}

CTEST2(os_notify_futex, os_notify_immediate_priority_selection) {
    // Set up multiple waiting threads with different priorities
    os_threads[1].state = THREAD_STATE_WAITING;
    os_threads[1].priority = 50;  // Lower priority (higher number)
    os_threads[1].wait_object = &futex;
    os_threads[1].wait_callback = cb_syscall_notify_object;
    
    os_threads[2].state = THREAD_STATE_WAITING;
    os_threads[2].priority = 10;  // Higher priority (lower number)  
    os_threads[2].wait_object = &futex;
    os_threads[2].wait_callback = cb_syscall_notify_object;
    
    os_threads[4].state = THREAD_STATE_WAITING;
    os_threads[4].priority = 30;  // Medium priority
    os_threads[4].wait_object = &futex;
    os_threads[4].wait_callback = cb_syscall_notify_object;
    
    int rv = os_sys_notify_object2(&futex, NOTIFY_PRIORITY_DROP);
    
    ASSERT_EQUAL(rv, E_OK);
    // Thread 2 (highest priority) should be notified
    ASSERT_NULL(os_threads[2].wait_object);
    ASSERT_NULL(os_threads[2].wait_callback);

    // Other threads should still be waiting
    ASSERT_NOT_NULL(os_threads[1].wait_object);
    ASSERT_NOT_NULL(os_threads[4].wait_object);
}

CTEST2(os_notify_futex, os_wait_object_value_multiple_waiters_same_object) {
    // Set up multiple threads waiting on same object with different priorities
    futex = 1;
    
    os_threads[1].state = THREAD_STATE_WAITING;
    os_threads[1].priority = 40;
    os_threads[1].wait_object = &futex;
    os_threads[1].wait_callback = cb_syscall_notify_object;
    
    os_threads[2].state = THREAD_STATE_WAITING;  
    os_threads[2].priority = 20;  // Highest priority
    os_threads[2].wait_object = &futex;
    os_threads[2].wait_callback = cb_syscall_notify_object;
    
    // Now notify immediate - should wake highest priority thread
    int rv = os_sys_notify_object2(&futex, NOTIFY_PRIORITY_DROP);
    
    ASSERT_EQUAL(rv, E_OK);
    ASSERT_NULL(os_threads[2].wait_object);  // High priority thread notified
    ASSERT_NOT_NULL(os_threads[1].wait_object);  // Low priority still waiting
}

CTEST2(os_notify_futex, os_wait_object_value_flags_parameter) {
    // Test different flag values (currently unused but should not cause errors)
    futex = 1;
    
    int rv1 = os_sys_wait_for_object_value(&futex, 0, 0, 0);
    ASSERT_EQUAL(rv1, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
    
    // Reset state for next test
    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[0].wait_object = NULL;
    os_threads[0].wait_callback = NULL;

    core[0].thread_current = 0;

    int rv2 = os_sys_wait_for_object_value(&futex, 0, 0, 0xFFFFFFFF);
    ASSERT_EQUAL(rv2, E_INVALID);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_RUNNING);
}

CTEST2(os_notify_futex, os_wait_object_value_increment_match) {
    // Test different flag values (currently unused but should not cause errors)
    futex = 1;
    int rv = os_sys_wait_for_object_value(&futex, 1, 0, NOTIFY_VALUE_INCREMENT);
    ASSERT_EQUAL(rv, E_OK_NO_WAIT);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_RUNNING);
    ASSERT_EQUAL(futex, 2);
}

CTEST2(os_notify_futex, os_wait_object_value_increment_fail) {
    // Test different flag values (currently unused but should not cause errors)
    futex = 1;
    int rv = os_sys_wait_for_object_value(&futex, 0, 0, NOTIFY_VALUE_INCREMENT);
    ASSERT_EQUAL(rv, E_OK);
    ASSERT_EQUAL(thread_state(0), THREAD_STATE_WAITING);
    ASSERT_EQUAL(futex, 1);
}
