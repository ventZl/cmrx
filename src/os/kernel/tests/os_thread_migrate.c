#include "cmrx/defines.h"
#include <kernel/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// These tests can only be executed on systems which have more than
// one physical core in SMP. On other systems, the necessary structures
// are not present in the kernel.

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];

extern int os_stack_create();
extern int os_thread_exit(int status);
extern int os_thread_migrate(uint8_t thread_id, int status);
extern bool schedule_context_switch_called;
extern unsigned cmrx_current_core;

CTEST_DATA(os_thread_migrate) {
};

CTEST_SETUP(os_thread_migrate) {
    memset(&os_stacks, 0, sizeof(os_stacks));
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].priority = 32;
    os_threads[1].priority = 32;
    os_threads[2].priority = 16;
    os_threads[3].priority = 64;
    os_threads[4].priority = 64;

    os_threads[0].core_id = 0;
    os_threads[1].core_id = 0;
    os_threads[2].core_id = 0;
    os_threads[3].core_id = 0;
    os_threads[4].core_id = 1;

    os_threads[0].state = THREAD_STATE_RUNNING;
    os_threads[1].state = THREAD_STATE_STOPPED;
    os_threads[2].state = THREAD_STATE_EMPTY;
    os_threads[3].state = THREAD_STATE_FINISHED;
    os_threads[4].state = THREAD_STATE_RUNNING;

    os_threads[0].stack_id = os_stack_create();
    os_threads[1].stack_id = os_stack_create();
    os_threads[4].stack_id = os_stack_create();

    schedule_context_switch_called = false;
    core[0].thread_current = 0;
    core[1].thread_current = 4;

}

CTEST2(os_thread_migrate, clean_migration_stopped_thread) {
    cmrx_current_core = 0;

    int return_value = os_thread_migrate(1, 1);
    ASSERT_EQUAL(return_value, E_OK);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_MIGRATING);
    ASSERT_EQUAL(os_threads[1].core_id, 1);

    os_thread_continue(1);
    ASSERT_EQUAL(os_threads[1].state, THREAD_STATE_READY);
    ASSERT_EQUAL(os_threads[1].core_id, 1);
}

CTEST2(os_thread_migrate, rejected_migration_empty_thread) {
    cmrx_current_core = 0;

    int return_value = os_thread_migrate(2, 1);
    ASSERT_EQUAL(return_value, E_INVALID);
    ASSERT_EQUAL(os_threads[2].state, THREAD_STATE_EMPTY);
    ASSERT_EQUAL(os_threads[2].core_id, 0);
}

CTEST2(os_thread_migrate, rejected_migration_foreign_thread) {
    cmrx_current_core = 0;

    // Attempt to migrate thread running on different core to local core
    int return_value = os_thread_migrate(4, 0);

    ASSERT_EQUAL(return_value, E_INVALID);
    ASSERT_EQUAL(os_threads[4].core_id, 1);
    ASSERT_EQUAL(os_threads[4].state, THREAD_STATE_RUNNING);
}

CTEST2(os_thread_migrate, rejected_migration_running_thread) {
    cmrx_current_core = 0;

    int return_value = os_thread_migrate(0, 1);
    ASSERT_EQUAL(return_value, E_INVALID);
    ASSERT_EQUAL(os_threads[0].state, THREAD_STATE_RUNNING);
    ASSERT_EQUAL(os_threads[0].core_id, 0);
}
