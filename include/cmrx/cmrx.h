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

extern void os_start();

/** @} */
