#include <cmrx/os/sanitize.h>
#include <cmrx/os/sched/stack.h>
#include <cmrx/assert.h>
#include <cmrx/os/sched.h>

extern struct OS_stack_t os_stacks;

void sanitize_psp(uint32_t * psp)
{
	uint8_t stack_current = os_get_current_stack();
//	ASSERT(&os_stacks.stacks[stack_current][0] <= psp && psp <= &os_stacks.stacks[stack_current][OS_STACK_DWORD]);

}
