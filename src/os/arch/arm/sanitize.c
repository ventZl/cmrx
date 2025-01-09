#include <kernel/sanitize.h>
#include <cmrx/assert.h>
#include <kernel/sched.h>

extern struct OS_stack_t os_stacks;

void sanitize_psp(uint32_t * psp)
{
    (void) psp;
	uint8_t stack_current = os_get_current_stack();
	ASSERT(&os_stacks.stacks[stack_current][0] <= psp && psp <= &os_stacks.stacks[stack_current][OS_STACK_DWORD]);
}

void sanitize_psp_for_thread(uint32_t * psp, Thread_t thread_id)
{
    (void) psp;
    struct OS_thread_t * thread = os_thread_get(thread_id);
    ASSERT(&os_stacks.stacks[thread->stack_id][0] <= psp && psp <= &os_stacks.stacks[thread->stack_id][OS_STACK_DWORD]);
}
