#pragma once

#include <kernel/rpc.h>
#include <RTE_Components.h>
#include CMSIS_device_header

struct OS_thread_t;

/** ARM-specific thread initialization stub.
 *
 * It will initialize FPU usage state to "not used" upon thread
 * creation so bulk of threads won't consume stack by reserving
 * space for FPU state being saved.
 */
#if __FPU_USED

void os_save_fpu_context(struct OS_thread_t * thread);
void os_load_fpu_context(struct OS_thread_t * thread);
bool os_fpu_exception_frame(void);

#define os_save_exc_return(thread)      thread->arch.exc_return = (uint32_t) __get_LR()
#define os_load_exc_return(thread)      __set_LR((void *) thread->arch.exc_return)

/* Figure out if thread is using FPU.
 * @return true if thread has FPU active, false otherwise
 */
bool os_is_thread_using_fpu(Thread_t thread_id);

/* Implementation of the porting layer API. */
void os_init_arch(void);
void os_init_core(unsigned core_id);

#else
// These functions are not needed if FPU is not enabled, turn them into no-op
#   define os_save_fpu_context(x)
#   define os_load_fpu_context(x)
#   define os_save_exc_return(thread)
#   define os_load_exc_return(thread)

#   define os_is_thread_using_fpu(thread) (false)
#   define os_fpu_exception_frame() (false)


#   define os_init_arch(x)
#   define os_init_core(x)
#endif


/** ARM-specific architecture state of a thread.
 * This structure holds additional state needed by the
 * ARM Cortex M CPUs to properly store/restore thread
 * state.
 *
 * As of now the floating point usage information is stored here.
 * This is provided only if FPU is available and activated.
 */

struct Arch_State_t {
#if __FPU_USED
    /** Exception return saved at the point PendSV switches tasks. Used to figure out how to restore the thread. */
    uint32_t exc_return;
#endif
};


