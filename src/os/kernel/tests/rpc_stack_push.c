#include <cmrx/os/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];
bool rpc_stack_push(Process_t process_id);

CTEST_DATA(rpc_stack_push) {

};

CTEST_SETUP(rpc_stack_push) {
    memset(&os_threads, 0, sizeof(os_threads));
    memset(&os_processes, 0, sizeof(os_processes));
    core[0].thread_current = 0;
}

CTEST2(rpc_stack_push, push_empty) {
    bool rv = rpc_stack_push(2);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 1);
    ASSERT_EQUAL(os_threads[0].rpc_stack[1], 2);
}

CTEST2(rpc_stack_push, push_more) {
    bool rv = rpc_stack_push(2);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 1);
    ASSERT_EQUAL(os_threads[0].rpc_stack[1], 2);

    rv = rpc_stack_push(3);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 2);
    ASSERT_EQUAL(os_threads[0].rpc_stack[2], 3);
}

CTEST2(rpc_stack_push, push_right_thread) {
    core[0].thread_current = 5;
    bool rv = rpc_stack_push(2);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(os_threads[5].rpc_stack[0], 1);
    ASSERT_EQUAL(os_threads[5].rpc_stack[1], 2);

    rv = rpc_stack_push(3);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(os_threads[5].rpc_stack[0], 2);
    ASSERT_EQUAL(os_threads[5].rpc_stack[2], 3);
}

CTEST2(rpc_stack_push, push_full) {
    core[0].thread_current = 5;
    os_threads[5].rpc_stack[0] = 8;
    bool rv = rpc_stack_push(2);

    ASSERT_EQUAL(rv, false);
    ASSERT_EQUAL(os_threads[5].rpc_stack[0], 8);
}


