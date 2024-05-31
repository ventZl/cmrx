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
 * @{
 */
#define E_OK					0
#define E_OUT_OF_RANGE			1
#define E_OUT_OF_STACKS			2
#define E_TASK_RUNNING			3
#define E_INVALID_ADDRESS		4
#define E_MISALIGNED			5
#define E_WRONG_SIZE			6
#define E_BUSY					7
#define E_DEADLK				8
#define E_NOTAVAIL				9
#define E_INVALID				10
#define E_IN_TOO_DEEP			11
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
/** @} */

