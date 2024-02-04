#pragma once

/** @defgroup api_shared Shared memory
 *
 * @ingroup api
 *
 * API to declare memory as shared between two processes.
 *
 * CMRX does not allow for unconditional sharing of memory between two processes. 
 * Memory can be shared only between two processes during the remote procedure call.
 * Even at this time, the process which serves the RPC does not have full access to 
 * the caller's memory. This is to ensure robustness. If client has a need 
 * to communicate larger chunk of data with RPC server, then the buffer holding this 
 * data must be marked as belonging to the shared memory.
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
