/** @defgroup config Configuration
 *
 * Various aspects of kernel can be configured at build time.
 */
/** @defgroup util Utilities
 */
/** @defgroup os Kernel internals
 *
 *
 */

#pragma once

#include <stdint.h>
#include "defines.h"

#include <conf/kernel.h>

#ifdef KERNEL_HAS_MEMORY_PROTECTION
#	include "mpu.h"
#endif

#define OS_TASK_MPU_REGION_CODE		0
#define OS_TASK_MPU_REGION_DATA		1
#define OS_TASK_MPU_REGION_BSS		2
#define OS_TASK_MPU_REGION_MMIO		3



