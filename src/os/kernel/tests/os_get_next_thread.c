#include <cmrx/os/sched.h>
#include <ctest.h>
#include <string.h>

// Private kernel method, not mentioned in any header file
extern bool os_get_next_thread(uint8_t current, uint8_t * next);

/* Test yield behavior with only one runnable thread.
 * Test if scheduler will behave as documented in case there is 
 * only one runnable thread.
 */
CTEST(sched, yield_single_thread) {
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[0].priority = 32;

    os_threads[1].state = THREAD_STATE_STOPPED;
    os_threads[1].priority = 24;

    // state is THREAD_STATE_EMPTY;
    os_threads[2].priority = 1;

    os_threads[3].state = THREAD_STATE_FINISHED;
    os_threads[4].priority = 128;

    uint8_t next_thread;
    bool rv = os_get_next_thread(0, &next_thread);

    ASSERT_EQUAL(rv, false);
    ASSERT_EQUAL(next_thread, 0);
}

/* Test if round robin time sharing works in scheduler.
 * Test if scheduler will periodically switch between two or
 * more threads having same priority.
 */
CTEST(sched, yield_same_priority) {
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[0].priority = 32;

    os_threads[1].state = THREAD_STATE_READY;
    os_threads[1].priority = 32;

    uint8_t next_thread;
    bool rv = os_get_next_thread(0, &next_thread);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(next_thread, 1);

    // simulate thread switch
    os_threads[0].state = THREAD_STATE_READY;
    os_threads[1].state = THREAD_STATE_RUNNING;

    rv = os_get_next_thread(1, &next_thread);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(next_thread, 0);
}

/* Test if scheduler will select thread of higher priority.
 * Test if scheduler will switch to higher priority thread and keep
 * it running.
 */
CTEST(sched, yield_higher_priority) {
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[0].priority = 32;

    os_threads[1].state = THREAD_STATE_READY;
    os_threads[1].priority = 16;

    uint8_t current_thread = 0;
    uint8_t next_thread;
    bool rv = os_get_next_thread(current_thread, &next_thread);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(next_thread, 1);

    // simulate thread switch
    os_threads[0].state = THREAD_STATE_READY;
    os_threads[1].state = THREAD_STATE_RUNNING;

    current_thread = 1;
    rv = os_get_next_thread(current_thread, &next_thread);

    ASSERT_EQUAL(rv, false);
    ASSERT_EQUAL(next_thread, 1);

}

/* Test if scheduler will ignore thread of lower priority.
 * Test if scheduler will ignore lower priority runnable thread and keep
 * running the higher priority thread instead.
 */
CTEST(sched_e, yield_lower_priority) {
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[0].priority = 16;

    os_threads[1].state = THREAD_STATE_READY;
    os_threads[1].priority = 32;

    uint8_t current_thread = 0;
    uint8_t next_thread;
    bool rv = os_get_next_thread(current_thread, &next_thread);

    ASSERT_EQUAL(rv, false);
    ASSERT_EQUAL(next_thread, 0);
}


