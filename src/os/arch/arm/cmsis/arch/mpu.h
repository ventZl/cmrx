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

/** @} */


