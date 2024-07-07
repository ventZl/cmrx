#include "background_checker.h"

#include <ctest.h>
#include <stdint.h>
#include <string.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/timer.h>
#include <arch/corelocal.h>

struct TimerEntry_t {
	uint32_t sleep_from;      ///< time at which sleep has been requested
	uint32_t interval;        ///< amount of time sleep shall take
	uint8_t thread_id;        ///< thread ID which requested the sleep
};

extern struct OS_stack_t os_stacks;
extern unsigned cmrx_os_smp_locked;

struct barrier_t * lock_barrier = NULL;
struct barrier_t * unlock_barrier = NULL;
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];
extern uint32_t sched_microtime;
extern struct OS_core_state_t core[OS_NUM_CORES];

void test_smp_locked_cb() {
    barrier_wait(lock_barrier);
}

void test_smp_unlocked_cb() {
    barrier_release(unlock_barrier);
}

CTEST_DATA(smp_atomic) {
};

CTEST_SETUP(smp_atomic) 
{
    (void) data;
    sched_microtime = 0;
	memset(&os_stacks, 0, sizeof(os_stacks));
    cmrx_smp_locked_callback = test_smp_locked_cb;
    cmrx_smp_unlocked_callback = test_smp_unlocked_cb;

    memset(&sleepers, 0, sizeof(sleepers));
    os_timer_init();
}

extern int os_stack_create();
extern void os_stack_dispose(uint32_t stack_id);

/** This test tests that when os_stack_create is called that:
 * - kernel SMP lock is locked exactly once
 * - before the lock is locked, the value of stack allocation bitmap is 0
 * - after the lock is unlocked, the value of stack allocation bitmap is 1
 */
CTEST2(smp_atomic, os_stack_alloc) {
    // Template steps
    // 1. Before os_stack_create is called
    // 2. After os_txn_start
    // 3. After os_txn_commit / done until os_stack_dispose is called
    uint32_t template[] = { 0, 0, 1 };

    struct checker_t * checker = checker_create(&os_stacks.allocations, template, sizeof(os_stacks.allocations), ARR_SIZE(template), &cmrx_os_smp_locked);
    lock_barrier = checker->lock_barrier;
    unlock_barrier = checker->unlock_barrier;

    int rv = os_stack_create();

    ASSERT_EQUAL(checker_finish(checker), OK);
    ASSERT_EQUAL(cmrx_os_smp_locked, 0);
    ASSERT_EQUAL(rv, 0);
}

CTEST2(smp_atomic, os_stack_dispose) {
    // Template steps
    // 1. Before os_stack_create is called
    // 2. After os_txn_start
    // 3. After os_txn_commit / done until os_stack_dispose is called
    // 4. After os_txn_start_commit
    uint32_t template[] = { 0, 0, 1, 0 };

    struct checker_t * checker = checker_create(&os_stacks.allocations, template, sizeof(os_stacks.allocations), ARR_SIZE(template), &cmrx_os_smp_locked);
    lock_barrier = checker->lock_barrier;
    unlock_barrier = checker->unlock_barrier;

    int rv = os_stack_create();

    ASSERT_EQUAL(cmrx_os_smp_locked, 0);
    ASSERT_EQUAL(rv, 0);

    os_stack_dispose(rv);

    ASSERT_EQUAL(checker_finish(checker), OK);
    ASSERT_EQUAL(cmrx_os_smp_locked, 0);
}

CTEST2(smp_atomic, os_setitimer) {
    static struct TimerEntry_t template[] = {{0, 0, 0xFF}, {0, 0, 0xFF}, {0, (1 << 31) | 10000, 4}};
    struct checker_t * checker = checker_create(&sleepers[0], template, sizeof(struct TimerEntry_t), ARR_SIZE(template), &cmrx_os_smp_locked);
    lock_barrier = checker->lock_barrier;
    unlock_barrier = checker->unlock_barrier;

    core[0].thread_current = 4;
    int rv = os_setitimer(10000);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(checker_finish(checker), OK);
}

CTEST2(smp_atomic, os_usleep) {
    static struct TimerEntry_t template[] = {{0, 0, 0xFF}, {0, 0, 0xFF}, {0, 10000, 4}, {0, 10000, 4}, {0, 10000, 4}};

    struct checker_t * checker = checker_create(&sleepers[0], template, sizeof(struct TimerEntry_t), ARR_SIZE(template), &cmrx_os_smp_locked);
    lock_barrier = checker->lock_barrier;
    unlock_barrier = checker->unlock_barrier;

    core[0].thread_current = 4;

    int rv = os_usleep(10000);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(checker_finish(checker), OK);

}
