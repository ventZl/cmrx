#include "background_checker.h"

#include <ctest.h>
#include <stdint.h>
#include <string.h>
#include <cmrx/os/runtime.h>
#include <arch/corelocal.h>

extern struct OS_stack_t os_stacks;
extern unsigned cmrx_os_smp_locked;

struct barrier_t * barrier = NULL;

void test_smp_locked_cb() {
    barrier_wait(barrier);
}

CTEST_DATA(smp_atomic) {
};

CTEST_SETUP(smp_atomic) 
{
    (void) data;
	memset(&os_stacks, 0, sizeof(os_stacks));
    cmrx_smp_locked_callback = test_smp_locked_cb;
    barrier = barrier_create();
}

extern int os_stack_create();
extern void os_stack_dispose(uint32_t stack_id);

/** This test tests that when os_stack_create is called that:
 * - kernel SMP lock is locked exactly once
 * - before the lock is locked, the value of stack allocation bitmap is 0
 * - after the lock is unlocked, the value of stack allocation bitmap is 1
 */
CTEST2(smp_atomic, os_stack_alloc) {
    uint32_t template[2] = { 0, 1 };

    struct checker_t * checker = checker_create(&os_stacks.allocations, template, sizeof(os_stacks.allocations), 2, &cmrx_os_smp_locked, barrier);

    int rv = os_stack_create();

    ASSERT_EQUAL(checker_finish(checker), OK);
    ASSERT_EQUAL(cmrx_os_smp_locked, 0);
    ASSERT_EQUAL(rv, 0);
}

CTEST2(smp_atomic, os_stack_dispose) {
    uint32_t template[3] = { 1, 0, 1 };

    struct checker_t * checker = checker_create(&os_stacks.allocations, template, sizeof(os_stacks.allocations), 3, &cmrx_os_smp_locked, barrier);

    int rv = os_stack_create();

    ASSERT_EQUAL(cmrx_os_smp_locked, 0);
    ASSERT_EQUAL(rv, 0);

    os_stack_dispose(rv);

    ASSERT_EQUAL(checker_finish(checker), OK);
    ASSERT_EQUAL(cmrx_os_smp_locked, 0);
}
