#include <kernel/sched.h>
#include <ctest.h>
#include <string.h>
#include <arch/corelocal.h>

// Kernel private functions and variables, not part of any header
extern struct OS_core_state_t core[OS_NUM_CORES];
int rpc_stack_pop();

CTEST_DATA(rpc_stack_pop) {

};

CTEST_SETUP(rpc_stack_pop) {
    memset(&os_threads, 0, sizeof(os_threads));
    memset(&os_processes, 0, sizeof(os_processes));
    core[0].thread_current = 0;
}

CTEST2(rpc_stack_pop, pop_empty) {
    int rv = rpc_stack_pop();

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 0);
}

CTEST2(rpc_stack_pop, pop_filled) {
    os_threads[0].rpc_stack[0] = 4;
    int rv = rpc_stack_pop();

    ASSERT_EQUAL(rv, 3);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 3);
}

CTEST2(rpc_stack_pop, pop_multiple) {
    os_threads[0].rpc_stack[0] = 3;
    int rv = rpc_stack_pop();

    ASSERT_EQUAL(rv, 2);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 2);

    rv = rpc_stack_pop();

    ASSERT_EQUAL(rv, 1);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 1);
    
    rv = rpc_stack_pop();

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 0);
    
    rv = rpc_stack_pop();

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(os_threads[0].rpc_stack[0], 0);
}


