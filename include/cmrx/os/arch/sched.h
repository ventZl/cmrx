#pragma once
/** @ingroup arch_arch
 * @{
 */

#include <cmrx/os/runtime.h>
#include <stdint.h>

/** Populate stack of new thread so it can be executed.
 * Populates stack of new thread so that it can be executed with no
 * other actions required. Returns the address where SP shall point to.
 * @param stack_id ID of stack to be populated
 * @param stack_size size of stack in 32-bit quantities
 * @param entrypoint address of thread entrypoint function
 * @param data address of data passed to the thread as its 1st argument
 * @returns Address to which the SP shall be set.
 */
uint32_t * os_thread_populate_stack(int stack_id, unsigned stack_size, entrypoint_t * entrypoint, void * data);

/** Create process using process definition.
 * Takes process definition and initializes MPU regions for process out of it.
 * @param process_id ID of process to be initialized
 * @param definition process definition. This is constructed at compile time using OS_APPLICATION macros
 * @returns E_OK if process was contructed properly, E_INVALID if process ID is already used or
 * if process definition contains invalid section boundaries. E_OUT_OF_RANGE is returned if process ID
 * requested is out of limits given by the size of process table.
 */
int os_process_create(Process_t process_id, const struct OS_process_definition_t * definition);

/** Start executing thread.
 * Used to actually start executing in thread mode just after the kernel
 * has been initialized and is ready to start the first thread. This function
 * has to perform CPU switch from privileged mode in which kernel runs into
 * unprivileged mode in which threads are supposed to run. Thread passed to
 * this function is in state ready to be executed by normal kernel thread
 * switching mechanism on this platform.
 * @param boot_thread ID of thread that shall be started
 */
__attribute__((naked,noreturn)) void os_boot_thread(Thread_t boot_thread);

/** @} */
