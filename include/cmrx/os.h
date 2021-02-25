#pragma once

#include <stdint.h>
#include "defines.h"

#include <conf/kernel.h>

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include "mpu.h"
#endif

#define OS_TASKS				4
#define OS_STACKS				4
#define OS_STACK_SIZE			0x100
#define OS_STACK_DWORD			(OS_STACK_SIZE/4)
#define OS_TASK_MPU_REGIONS		4

#define OS_TASK_MPU_REGION_CODE		0
#define OS_TASK_MPU_REGION_DATA		1
#define OS_TASK_MPU_REGION_BSS		2
#define OS_TASK_MPU_REGION_MMIO		3

#define TASK_STATE_EMPTY		0
#define TASK_STATE_READY		1
#define TASK_STATE_RUNNING		2
#define TASK_STATE_BLOCKED		3
#define TASK_STATE_CREATED		4

#define OS_TASK_NO_STACK		(~0)

typedef void (entrypoint_t)();

struct OS_task_t;

struct OS_thread_t {
	uint32_t * sp;
	uint32_t stack_id;
	uint32_t state;
#ifdef KERNEL_HAS_MEMORY_PROTECTION
	MPU_State mpu;
#endif
	const struct OS_task_t * task;
};

struct OS_stack_t {
	uint32_t allocations;
	uint32_t stacks[OS_STACKS][OS_STACK_DWORD];
};

extern struct OS_thread_t os_threads[OS_TASKS];

struct OS_MPU_region {
	void * start;
	void * end;
};

struct OS_task_t {
	entrypoint_t * entrypoint;
	struct OS_MPU_region mpu_regions[OS_TASK_MPU_REGIONS];

	void * interface_start; /* this is not an actual MPU region */
	void * interface_end;
};



#define OS_TASK_WORKER(entrypoint) \
{ \
	~0,\
	OS_TASK_NO_STACK,\
	TASK_STATE_CREATED,\
	entrypoint\
}

#define OS_TASK_INITIALIZED(entrypoint, stack_id) \
{ \
	&os_stacks.stacks[stack_id][OS_STACK_DWORD - 16],\

