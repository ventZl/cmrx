#pragma once

/** @addtogroup arch_arch
 * @{
 */

/** Finalize architecture-specific initialization of thread.
 * This routine is called when thread has been created. It has opportunity to
 * perform architecture-specific default-initialization of the thread.
 * @note To save some CPU cycles, you can define this function as an empty macro if it is not used.
 * In that case the @ref Arch_State_t should be an empty structure.
 */
extern void os_thread_initialize_arch(struct OS_thread_t * thread);

/** Architecture code-private part of thread record.
 * This structure holds the CPU-specific part of thread
 * state. It can be empty if architecture does not need
 * any specific information saved in the CPU state.
 */
struct Arch_State_t {
};

/** Perform architecture-specific initialization during kernel startup.
 * This can be used to perform system-wide initialization that has to be done once
 * per whole system.
 * Kernel guarantees that this function will be called exactly once
 */
extern void os_init_arch(void);

/** Perform architecture-specific initialization of core during boot.
 * This can be used to perform architecture specific initialization of CPU core
 * before kernel starts executing on the core.
 * Note that you shall not use this function to initialize memory protection unless
 * some very specific steps have to be taken as there is API to initialize MPU
 * elsewhere in the porting layer.
 * This function will be called once per core managed by the kernel before
 * kernel starts initialization of the kernel structures for that core.
 */
extern void os_init_core(unsigned core_id);

/** @} */
