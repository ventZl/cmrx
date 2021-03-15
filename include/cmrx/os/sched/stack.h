#pragma once

#include <stdint.h>
#include <conf/kernel.h>

#define OS_TASK_NO_STACK		(~0)
#define OS_STACK_DWORD			(OS_STACK_SIZE/4)

/** Kernel structure for maintaining thread stacks.
 *
 * Kernel allocates thread stacks here. Amount and size of
 * stacks can be configured using conf/kernel.h.
 */
struct OS_stack_t {
	/** Thread stacks. */
	uint32_t stacks[OS_STACKS][OS_STACK_DWORD];

	/** Information about stack allocation. If n-th bit is set, 
	 * then n-th stack is allocated. Otherwise it is available. */
	uint32_t allocations;
};

