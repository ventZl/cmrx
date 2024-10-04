#pragma once

/** @defgroup arch_arm_mpu Memory protection
 *
 * @ingroup arch_arm 
 *
 * Memory protection support internals for ARM architecture
 * @{ 
 */
#include <RTE_Components.h>
#include CMSIS_device_header

#include <kernel/arch/mpu.h>

#ifdef __ARM_ARCH_6M__

// ARM v6M doesn't have any of these
// provide some defaults which will make the code happy

#define SCB_CFSR			0
#define SCB_CFSR_IACCVIOL	1
#define SCB_CFSR_DACCVIOL	1
#define SCB_CFSR_MMARVALID	1
#define SCB_MMFAR			0

#endif

/** @defgroup arm_mpu_registers ARM MPU register aliases
 * @{
 */
#define MPU_CTRL (MPU->CTRL)
#define MPU_RNR (MPU->RNR)
#define MPU_RASR (MPU->RASR)
#define MPU_RBAR (MPU->RBAR)

/** @} */

#define MPU_RNR_REGION (MPU_RNR_REGION_Msk)
#define MPU_RNR_REGION_LSB (MPU_RNR_REGION_Pos)

/** @defgroup arm_mpu_rars ARM MPU RASR fields
 * @{ 
 */
#define MPU_RASR_ENABLE (MPU_RASR_ENABLE_Msk)
#define MPU_RASR_SIZE (MPU_RASR_SIZE_Msk)
#define MPU_RASR_SIZE_LSB (MPU_RASR_SIZE_Pos)
#define MPU_RASR_SRD (MPU_RASR_SRD_Msk)
#define MPU_RASR_SRD_LSB (MPU_RASR_SRD_Pos)
#define MPU_RASR_ATTR_C (MPU_RASR_C_Msk)
#define MPU_RASR_ATTR_XN (MPU_RASR_XN_Msk)
#define MPU_RASR_ATTR_AP (MPU_RASR_AP_Msk)
#define MPU_RASR_ATTR_AP_PRW_URO (ARM_MPU_AP_URO << MPU_RASR_AP_Pos)
#define MPU_RASR_ATTR_AP_PRW_URW (ARM_MPU_AP_FULL << MPU_RASR_AP_Pos)
/** @} */

/** @defgroup arm_mpu_rbar ARM MPU RBAR fields
 * @{ 
 */
#define MPU_RBAR_VALID (MPU_RBAR_VALID_Msk)
#define MPU_RBAR_ADDR (MPU_RBAR_ADDR_Msk)
#define MPU_RBAR_REGION (MPU_RBAR_REGION_Msk)
#define MPU_RBAR_REGION_LSB (MPU_RBAR_REGION_Pos)
/** @} */

/** @defgroup arm_mpu_ctrl ARM MPU CTRL fields
 * @{ 
 */
#define MPU_CTRL_ENABLE (MPU_CTRL_ENABLE_Msk)
#define MPU_CTRL_PRIVDEFENA (MPU_CTRL_PRIVDEFENA_Msk)
/** @} */

#define MPU_AP_MASK			0b0111
#define MPU_EXECUTE_SHIFT	3

/** @defgroup mpu_region_names MPU region names 
 * @ingroup arch_arm_mpu
 *
 * Names of ARM MPU regions as being used by CMRX.
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

/** Configure and activate MPU region.
 * Activate given memory region with new base address and size.
 * For now, caller is responsible for providing base address, which is aligned
 * to size of block. If region is already activated, then its base address,
 * size and flags are being overwritten.
 * @param region ID of region being activated (0-7)
 * @param base base address of region
 * @param size size of region (256B and more)
 * @param cls region access class, see @ref MPU_class for available access classes
 * @return E_OK if region was configured, otherwise error code is returned
 */
int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint8_t cls);

/** Create configuration for MPU region.
 *
 * @TODO
 */
int mpu_configure_region(uint8_t region, const void * base, uint32_t size, uint8_t flags, uint32_t * RBAR, uint32_t * RASR);

/** Disable MPU region.
 * This function will disable use of MPU region. Address and size
 * will remain configured, but region is not marked as enabled anymore.
 * @param region ID of region being deactivated (0 - 7)
 * @return E_OK if region was deactivated
 */
int mpu_clear_region(uint8_t region);


int mpu_load(const MPU_State * state, uint8_t base, uint8_t count);

/** @} */

