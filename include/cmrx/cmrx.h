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
 *
 * Give userspace applications access to the current CPU frequency.
 *
 * @returns CPU frequency in Hz or 0 if frequency is unknown or unreliable.
 */
__SYSCALL long get_cpu_freq(void);

/** @} */
