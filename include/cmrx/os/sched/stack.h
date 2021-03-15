#pragma once

#include <stdint.h>
#include <conf/kernel.h>

#define OS_TASK_NO_STACK		(~0)
#define OS_STACK_DWORD			(OS_STACK_SIZE/4)


struct OS_stack_t {
	uint32_t stacks[OS_STACKS][OS_STACK_DWORD];
	uint32_t allocations;
};

