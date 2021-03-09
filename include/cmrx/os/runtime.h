#pragma once

#include <cmrx/mpu.h>

enum ThreadState {
	THREAD_STATE_EMPTY = 0,
	THREAD_STATE_READY,
	THREAD_STATE_RUNNING,
	THREAD_STATE_CREATED,
	THREAD_STATE_STOPPED,
	THREAD_STATE_FINISHED,
	THREAD_STATE_BLOCKED_JOINING
};

typedef int (entrypoint_t)(void *);

struct OS_process_t;

struct OS_thread_t {
	uint32_t * sp;
	uint8_t stack_id;
	enum ThreadState state;
	uint32_t block_object;
	uint8_t rpc_depth;
	uint8_t priority;
	void (*signal_handler)(int);
	int exit_status;
#ifdef KERNEL_HAS_MEMORY_PROTECTION
	MPU_State mpu;
#endif
	const struct OS_process_t * process;
};

struct OS_MPU_region {
	void * start;
	void * end;
};

struct OS_process_t {
	struct OS_MPU_region mpu_regions[OS_TASK_MPU_REGIONS];

	void * interface_start; /* this is not an actual MPU region */
	void * interface_end;
};

struct OS_thread_create_t {
	const struct OS_process_t * process;
	entrypoint_t * entrypoint;
	void * data;
	uint8_t priority;
};

extern struct OS_thread_t os_threads[OS_THREADS];

