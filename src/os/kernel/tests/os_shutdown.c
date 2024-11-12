#include <kernel/sched.h>
#include <ctest.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>

extern unsigned cmrx_os_core_lock_called;
extern bool memory_protection_stop_called;
extern bool kernel_shutdown_called;

extern int os_stack_create();


CTEST_DATA(os_shutdown) {
};

CTEST_SETUP(os_shutdown) {
    (void) data;
    memset(&os_stacks, 0, sizeof(os_stacks));
    memset(&os_threads, 0, sizeof(os_threads));

    os_threads[0].priority = 32;

    os_threads[0].state = THREAD_STATE_RUNNING;

    os_threads[0].stack_id = os_stack_create();
    os_threads[1].stack_id = os_stack_create();

    core[0].thread_current = 0;
}

jmp_buf buf;

void cmrx_shutdown_handler()
{
    longjmp(buf, 1);
}

CTEST2(os_shutdown, normal_shutdown)
{
    if (setjmp(buf) == 0)
    {
        os_shutdown();
    }

    // To test that interrupts were disabled by the kernel
    ASSERT_EQUAL(cmrx_os_core_lock_called, 1);
    // To test that the call to disable memory protection has been made
    ASSERT_EQUAL(memory_protection_stop_called, true);
    // To test that the kernel shutdown has been called
    ASSERT_EQUAL(kernel_shutdown_called, true);
}
