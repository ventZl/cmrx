#pragma once

/** @defgroup api_shared Shared memory
 *
 * @ingroup api
 *
 * During remote procedure call, called procedure has no access to memory area
 * of calling thread / process. This is to ensure robustness. If caller needs
 * to communicate larger chunk of data with RPC callee, then it must be passed
 * either via stack or using shared memory section.
 */

/** @ingroup api_shared
 * @{
 */

/** Mark variable as being shared with RPC callees.
 * If this specifier is used while defining a variable, then
 * this variable will be reachable from within RPC call routine.
 * Effectively this specifier moves given variable into 
 * process's shared section, which remains mapped in address
 * space even during RPC call is in progress.
 */
#define SHARED __attribute__((section(".shared")))

/** @} */
