#pragma once

#include <conf/kernel.h>
#include <stdint.h>

/** Convenience structure which groups RBAR and RASR register 
 * for one MPU region together
 */
struct MPU_Registers {
	uint32_t _MPU_RBAR;
	uint32_t _MPU_RASR;
};

/** Type handling MPU state as remembered by CMRX thread switcher
 */
typedef struct MPU_Registers MPU_State[MPU_STATE_SIZE];

#define MPU_AP_MASK			0b0111
#define MPU_EXECUTE_SHIFT	3

/** @defgroup mpu_region_names MPU region names 
 * @{
 */
/// Region for initialized readable/writable data 
#define OS_MPU_REGION_DATA			0
/// Region for uninitialized readable/writable data
#define OS_MPU_REGION_BSS			1
/// Region covering memory-mapped IO devices
#define OS_MPU_REGION_MMIO			2
/// Region containing shared/sharable resources
#define OS_MPU_REGION_MMIO2 		3
/// Currently unused region (reserved)
#define OS_MPU_REGION_SHARED		4
/// Currently unused region (reserved)
#define OS_MPU_REGION_UNUSED2		5
/// Region covering thread's stack
#define OS_MPU_REGION_STACK			6
/// Region covering executable RAM (?)
#define OS_MPU_REGION_EXECUTABLE	7
/** @} */


