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
#ifdef __FPU_USED
void os_thread_initialize_arch(struct OS_thread_t * thread);
#else
#   define os_thread_initialize_arch(x)
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
#ifdef __FPU_USED
    /** A bitmask carrying information if any of RPC levels was actively using FPU. */
    uint8_t fp_active;
#endif
};

/** Provide information on if FPU is used by certain thread.
 * This provides information on if FPU is used by certain thread and
 * RPC call level. In CMRX the ABI boundary between the caller and callee
 * is integer only. FPU context is not carried over the rpc_call/rpc_return boundary.
 * @returns true if topmost execution context within thread is using FPU. False otherwise.
 */
bool os_is_fpu_active_in_thread(Thread_t thread_id);
