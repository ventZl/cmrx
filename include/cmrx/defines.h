/** @defgroup api Kernel API
 *
 * Public API for userspace code to interface with the kernel.
 *
 * This API provides access to all services of the CMRX RTOS and covers all callbacks CMRX 
 * might have into integrators's code. This API is designed to be cross-platform compatible
 * and from the userspace point of view, it is presented as functions that can be called.
 *
 * For certain purposes, mainly for customization, CMRX might require integrator to provide
 * some callbacks that CMRX can call. These callbacks are also cross-platform compatible and
 * allow the integrator to customize some basic services such as clock source and power
 * management.
 *
 * @{
 */
#pragma once

#include <stdint.h>

/** @defgroup api_errors Named constants for errors
 * Types of errors system calls in CMRX can return.
 * @{
 */
/// No error. Operation completed successfully
#define E_OK					0
/// Object ID passed by the caller is out of range
#define E_OUT_OF_RANGE			1
/// There is no stack space to be allocated for the thread
#define E_OUT_OF_STACKS			2
/// Internal error. Suggests that operation is made on thread that is already running while the semantics of the operation suggests is should not be
#define E_TASK_RUNNING			3
/// Internal error. Invalid address has been passed by the caller
#define E_INVALID_ADDRESS		4
/// Internal error. MPU region is misaligned and can't be configured into the hardware
#define E_MISALIGNED			5
/// Internal error. MPU region size is wrong.
#define E_WRONG_SIZE			6
/// Object is busy and can't complete the requested operation
#define E_BUSY					7
/// Deadlock detected
#define E_DEADLK				8
/// Object or service is not available
#define E_NOTAVAIL				9
/// Caller passed generally invalid argument
#define E_INVALID				10
/// RPC call cannot be dispatched because this thread is already too deep in the RPC call tree
#define E_IN_TOO_DEEP			11
/// Too many pending notifications for differnt objects
#define E_OUT_OF_NOTIFICATIONS  12
/// Scheduler yield is required
#define E_YIELD                 13
/// Operation timed out
#define E_TIMEOUT               14
/// Internal error. No more free threads
#define E_OUT_OF_THREADS        0xFF
/** @} */

/** Name the null pointer.
 * NULL is not a C name, rather than POSIX one. Introduce it here so we can
 * have a named null pointer rather than a magic constant.
 */
#ifndef NULL
#   define NULL					((void *) 0)
#endif

/** Data type used for thread IDs */
typedef uint8_t Thread_t;

/** Data type used for process IDs */
typedef uint8_t Process_t;

/** @} */

