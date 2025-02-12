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
void os_thread_initialize_arch(struct OS_thread_t * thread);

/** Architecture code-private part of thread record.
 * This structure holds the CPU-specific part of thread
 * state. It can be empty if architecture does not need
 * any specific information saved in the CPU state.
 */
struct Arch_State_t {
};

/** @} */
