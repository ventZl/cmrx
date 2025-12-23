#pragma once

#include <conf/kernel.h>
#include <stdint.h>

/** @defgroup arch_arm_mpu Memory protection
 *
 * @ingroup arch_arm
 *
 * Memory protection support internals for ARM architecture
 * @{
 */
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)

/** ARMv8M MPU registers for one region (base + limit model)
 */
struct MPU_Registers {
	uint32_t _MPU_RBAR;  /* Region Base Address Register */
	uint32_t _MPU_RLAR;  /* Region Limit Address Register */
};

#else

/** ARMv6M/ARMv7M MPU registers for one region (base + size model)
 */
struct MPU_Registers {
	uint32_t _MPU_RBAR;  /* Region Base Address Register */
	uint32_t _MPU_RASR;  /* Region Attribute and Size Register */
};

#endif

/** Type handling MPU state as remembered by CMRX thread switcher
 */
typedef struct MPU_Registers MPU_State[MPU_STATE_SIZE];

/** @} */


