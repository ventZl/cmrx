#pragma once

#include <cmrx/sys/runtime.h>
#include <cmrx/defines.h>
#include <conf/kernel.h>
#include <arch/runtime.h>

/** @defgroup os_kernel Kernel core
 * @ingroup os
 * Basic structures that contain the internal state of kernel.
 * Kernel is built around minimalistic core, which consists of prioritized thread
 * scheduler. Scheduler can be started up after basic HW setup is done by calling
 * @ref os_start(). This will collect all auto-started threads, prepare them and
 * start thread scheduler. All other mechanisms are built around thread scheduler.
 * @{
 */

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
	 * @ref os_thread_continue().
	 */
	THREAD_STATE_STOPPED,
	/** Thread finished it's execution either explicitly by calling @ref thread_exit()
	 * or by returning from thread entrypoint. Nobody called @ref thread_join() yet.
	 * This state is the same as "zombie" in Linux.
	 */
	THREAD_STATE_FINISHED,
	/** Thread is waitihg for object to be notified
	 */
	THREAD_STATE_WAITING,
    /** Thread has been stopped and is ready to migrate over to another core
     */
    THREAD_STATE_MIGRATING
};


/** RPC call owner process stack.
 * This stack records owners of nested RPC calls. It can accomodate up to 8 owners
 * which means 8 nested RPC calls.
 */
typedef Process_t OS_RPC_stack[8];

/** Type to carry event being notified.
 * Event signals what exactly happened to the object being notified. See @ref EventTypes for
 * the list of events.
 */
typedef int Event_t;

/** Prototype for function that handles thread wakeup on notification.
 */
typedef void (WaitHandler_t)(const void *, Thread_t, int, Event_t);



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
	/** Identification of object, which causes this thread to block.
	 * This value if context-dependent. If thread is blocked joining other thread,
	 * then this contains thread ID. If thread is blocked waiting for mutex, then
	 * this contains mutex address.
	 */
	const void * wait_object;

	WaitHandler_t * wait_callback;

	/** Address of signal handler to use.
	 */
	void (*signal_handler)(int);

	/** Current pending signal bitmask.
	 * Each set bit means that one signal has been delivered.
	 */

	uint32_t signals;

	struct Arch_State_t arch;

	/** Exit status after thread quit. */
	int exit_status;

	/** Ummmmm... */
	OS_RPC_stack rpc_stack;

	/** ID of stack, which is allocated to this thread. */
	uint8_t stack_id;

	/** State of this thread. */
	enum ThreadState state;

	/** Thread priority.
	 * This is used by scheduler to decide which thread to run.
	 */
	uint8_t priority;

	/** Owning process reference. */
    Process_t process_id;

    /** Core at which this thread is running */
    uint8_t core_id;
};

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

/** Process control block.
 *
 * Process is a container whose sole purpose is to drive the MPU.
 * All threads, which are bound to the same process, share common access rights
 * to memory using MPU regions. By default, threads bound to different processes
 * can't access each other's memory. Use shared memory for this.
 */
struct OS_process_t {
	const struct OS_process_definition_t * definition;

#ifdef KERNEL_HAS_MEMORY_PROTECTION
	/** Memory protection registers dump. This is only valid in cases, when
	 * @ref sp is valid.
	 */
	MPU_State mpu;
#endif

};



/** Scheduler notion on existing threads. */
extern struct OS_thread_t os_threads[OS_THREADS];

/** Scheduler notion on existing processes. */
extern struct OS_process_t os_processes[OS_PROCESSES];

/** Scheduler notion on existing stacks. */
extern struct OS_stack_t os_stacks;


/** Structure holding current scheduling state of CPU.
 *
 * This structure holds complete state required to perform thread scheduling.
 * It is private to CPU. Kernel should allocate as many of these as there are
 * CPUs.
 */
struct OS_core_state_t {
	Thread_t thread_prev; ///< previously running thread ID
	Thread_t thread_current; ///< ID of thread which is currently executed
	Thread_t thread_next; ///< ID of thread which will be scheduled once thread switch occurs.
};


/** @} */
