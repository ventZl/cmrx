#pragma once

/** @defgroup os Kernel documentation
 *
 * Documentation of kernel internals.
 * 
 * Covers parts of CMRX running in the privileged mode. Most of this code is accessible
 * via system calls, while some being triggered by timing provider API or occupying vital
 * system handlers. Kernel is divided into two parts: platform-independent part and part
 * that is ported to support individual platforms. 
 *
 * Documentation of kernel internals is usable if you want to understand how certain
 * features are implemented or if you are porting CMRX to another platform.
 * @{
 */

#include <stdint.h>
#include <conf/kernel.h>
#include <arch/sysenter.h>

#ifdef CMRX_ARCH_SMP_SUPPORTED
#   define os_start(core) _os_start((core))
#else
#   define os_start() _os_start(0)
#endif

// Declaring this as noreturn will break HIL tests
extern void _os_start(uint8_t core);

/** @} */

/** @defgroup api_standard Standard API
 * @ingroup api
 * API providing standardized access to various aspects required by userland.
 */

/**
 * @ingroup api_standard
 * @{ */

/** Obtain the current CPU frequency.
 * Give userspace applications access to the current CPU frequency.
 *
 * @returns CPU frequency in Hz or 0 if frequency is unknown or unreliable.
 */
__SYSCALL long get_cpu_freq(void);

/** Obtain the current scheduler time.
 * Returns current scheduler time. The time is returned with nanosecond
 * resolution and precision depending on the timing provider implemented.
 *
 * This value wraps around after about 4000 seconds on 32-bit CPUs
 *
 * @returns current scheduler time in nanoseconds
 */
__SYSCALL uint32_t getmicrotime(void);

/** Shutdown CMRX kernel
 *
 * This syscall will initiate CMRX kernel shutdown. Kernel shutdown ensures
 * that the timing provider is stopped. Interrupts are also disabled globally.
 * No check on existing threads status is made. It is up to the application
 * developer to ensure that threads are in stable state before kernel is shut
 * down. Any work done and not saved in threads still active will be lost.
 *
 * It is not possible to call CMRX system calls once kernel has been shut down.
 * While in general, the possibility of restarting kernel after it has been
 * shut down is not ruled out this workflow is considered as unsupported.
 *
 * After kernel is shut down the execution will continue in function named
 * cmrx_shutdown_handler(). If this function is not defined, then the CPU will
 * be reset.
 *
 * CPU state when cmrx_shutdown_handler() is called:
 * - memory protection disabled
 * - privileged mode execution
 * - all interrupts disabled
 */

__SYSCALL uint32_t shutdown(void);

/** @} */
