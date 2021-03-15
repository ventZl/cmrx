#pragma once

#include <cmrx/mpu.h>

/** List of states in which thread can be.
 */
enum ThreadState {
	/// This thread slot is empty (default after reset)
	THREAD_STATE_EMPTY = 0,
	/// Thread is ready to be scheduled
	THREAD_STATE_READY,
	/// Thread is currently running
	THREAD_STATE_RUNNING,
	/// Thread was created, but does not have stack assigned
	THREAD_STATE_CREATED,
	/** Thread was running, but was forced to stop. It still has valid state so can't be
	 * disposed of without damage. Can be placed into ready/running state by calling
	 * @ref thread_continue().
	 */
	THREAD_STATE_STOPPED,
	/** Thread finished it's execution either explicitly by calling @ref thread_exit()
	 * or by returning from thread entrypoint. Nobody called @ref thread_join() yet.
	 * This state is the same as "zombie" in Linux.
	 */
	THREAD_STATE_FINISHED,
	/** Thread is blocked waitihg for other thread to finish.
	 */
	THREAD_STATE_BLOCKED_JOINING
};

/** Prototype for thread entrypoint function.
 * Thread entrypoint function takes one user-defined argument. By default it is assumed
 * that this is a pointer to user data, but there is no checking performed on the value
 * passed to the callee. It can be anything, once properly typecasted.
 * Thread entrypoint can return signed 32bit value as it's return value. This is available
 * for whoever will call @ref thread_join() as thread exit status.
 */
typedef int (entrypoint_t)(void *);

struct OS_process_t;

/** Thread control block.
 *
 * This structure holds current status of the thread.
 */
struct OS_thread_t {
	/** Value of SP. This is only valid if thread is in state different than
	 * THREAD_STATE_RUNNING. Obviously it is undefined for empty thread slots
	 * and slots which don't have stack allocated yet.
	 */
	uint32_t * sp;
	/** ID of stack, which is allocated to this thread. */
	uint8_t stack_id;

	/** State of this thread. */
	enum ThreadState state;

	/** Identification of object, which causes this thread to block.
	 * This value if context-dependent. If thread is blocked joining other thread,
	 * then this contains thread ID. If thread is blocked waiting for mutex, then
	 * this contains mutex address.
	 */
	uint32_t block_object;

	/** Ummmmm... */
	uint8_t rpc_depth;

	/** Thread priority.
	 * This is used by scheduler to decide which thread to run.
	 */
	uint8_t priority;

	/** Address of signal handler to use.
	 */
	void (*signal_handler)(int);

	/** Exit status after thread quit. */
	int exit_status;
#ifdef KERNEL_HAS_MEMORY_PROTECTION
	/** Memory protection registers dump. This is only valid in cases, when
	 * @ref sp is valid.
	 */
	MPU_State mpu;
#endif
	/** Owning process reference. */
	const struct OS_process_t * process;
};

/** MPU region description.
 */
struct OS_MPU_region {
	/** Start address. */
	void * start;
	/** End address. */
	void * end;
};

/** Process control block.
 *
 * Process is a container whose sole purpose is to drive the MPU.
 * All threads, which are bound to the same process, share common access rights
 * to memory using MPU regions. By default, threads bound to different processes
 * can't access each other's memory. Use shared memory for this.
 */
struct OS_process_t {
	/** Static MPU region configuration for this process.
	 */
	struct OS_MPU_region mpu_regions[OS_TASK_MPU_REGIONS];

	/** Ummmm */
	void * interface_start; /* this is not an actual MPU region */
	/** Ummmm 2 */
	void * interface_end;
};

/** Structure describing auto-spawned thread.
 *
 * This is a mechanism of creating threads without need to explicitly call
 * @ref thread_create(). Kernel will do that automatically upon main calling
 * @ref os_start().
 */
struct OS_thread_create_t {
	/** Owning process */
	const struct OS_process_t * process;

	/** Entrypoint address */
	entrypoint_t * entrypoint;

	/** User data passed to entrypoint function */
	void * data;

	/** Thread priority */
	uint8_t priority;
};

/** Scheduler notion on existing threads. */
extern struct OS_thread_t os_threads[OS_THREADS];

