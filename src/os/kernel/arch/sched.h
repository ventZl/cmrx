#pragma once
/** @ingroup arch_arch
 * @{
 */

#include <kernel/runtime.h>
#include <stdint.h>

/** Perform architecture-specific thread initialization
 * This function should perform the following actions:
 * - Configure thread state so it can be executed immediately after this function is done
 * - Set valid value into SP member, so the stack pointer can be set to valid value
 * - Perform any architecture-specific steps needed to make thread executable
 * - Set any architecture-specific settings
 * @param thread pointer to structure describing the thread just being created
 * @param stack_size size of stack in 32-bit quantities
 * @param entrypoint address of thread entrypoint function
 * @param data address of data passed to the thread as its 1st argument
 * @returns Address to which the SP shall be set.
 */
void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data);

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
__attribute__((noreturn)) void os_boot_thread(Thread_t boot_thread);

/** Stop running the kernel.
 * Return to bare metal execution mode similar to one after CPU reset.
 * This function should configure the CPU to continue execution in privileged mode
 * not distinguishing between thread and kernel space. Once this mode is configured
 * the function @ref cmrx_shutdown_handler() should be executed.
 *
 * This is a point of no return. Code here is free to destroy any previous
 * context.
 */
void os_kernel_shutdown();

/** Reset the CPU.
 * This is architecture- (and possibly HAL-) specific way to reset the CPU.
 * This function can be used before the kernel has been started or after it has been
 * shut down. If there is no shutdown handler provided by the integrator then
 * the default handler will call this function to reset the CPU automatically.
 */
__attribute__((noreturn)) void os_reset_cpu();

/** Set return value of syscall for given thread.
 * @param thread_id Id of the thread whose exception handler is being modified
 * @param retval value to store on the stack
 */
int os_set_syscall_return_value(Thread_t thread_id, int32_t retval);

/** @} */
