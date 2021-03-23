/** @defgroup api Kernel API
 *
 * Following documents public API available for userspace threads.
 *
 * @{
 */
#pragma once

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
/** @} */

#define NULL					((void *) 0)

/** @} */

