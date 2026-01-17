#pragma once

#include <RTE_Components.h>
#include CMSIS_device_header

#include <kernel/arch/mpu.h>

/** @defgroup arch_arm_mpu Memory protection
 *
 * @ingroup arch_arm
 *
 * Memory protection support internals for ARM architecture
 * @{
 */
#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_8M_BASE__)

// ARMv6M and ARMv8M-Baseline don't have detailed fault status registers
// provide some defaults which will make the code happy

#define SCB_CFSR			0
#define SCB_CFSR_IACCVIOL	1
#define SCB_CFSR_DACCVIOL	1
#define SCB_CFSR_MMARVALID	1
#define SCB_MMFAR			0

#endif

/** @} */

#define MPU_RNR_REGION (MPU_RNR_REGION_Msk)
#define MPU_RNR_REGION_LSB (MPU_RNR_REGION_Pos)

/** @defgroup arm_mpu_rars ARM MPU RASR fields (ARMv6M/ARMv7M)
 * @{
 */
#if !defined(__ARM_ARCH_8M_BASE__) && !defined(__ARM_ARCH_8M_MAIN__)
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
#endif
/** @} */

/** @defgroup arm_mpu_rlar ARM MPU RLAR fields (ARMv8M)
 * @{
 */
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
#define MPU_RLAR_ENABLE (MPU_RLAR_EN_Msk)
#define MPU_RLAR_LIMIT (MPU_RLAR_LIMIT_Msk)
#define MPU_RLAR_LIMIT_LSB (MPU_RLAR_LIMIT_Pos)
#define MPU_RLAR_ATTRINDX (MPU_RLAR_AttrIndx_Msk)
#define MPU_RLAR_ATTRINDX_LSB (MPU_RLAR_AttrIndx_Pos)

/* Memory attribute indices for MAIR */
#define MPU_ATTR_DEVICE_nGnRnE     0  /* Device memory, non-gathering, non-reordering, no early write ack */
#define MPU_ATTR_NORMAL_WT         1  /* Normal memory, write-through, read-allocate */
#define MPU_ATTR_NORMAL_WB         2  /* Normal memory, write-back, read/write-allocate */
#define MPU_ATTR_NORMAL_NC         3  /* Normal memory, non-cacheable */

/* Access permission encoding for RBAR */
#define MPU_RBAR_AP (MPU_RBAR_AP_Msk)
#define MPU_RBAR_AP_LSB (MPU_RBAR_AP_Pos)
#define MPU_RBAR_AP_RW_RW (0x0 << MPU_RBAR_AP_LSB)  /* Privileged RW, Unprivileged RW */
#define MPU_RBAR_AP_RW_RO (0x2 << MPU_RBAR_AP_LSB)  /* Privileged RW, Unprivileged RO */
#define MPU_RBAR_AP_RW_NONE (0x1 << MPU_RBAR_AP_LSB)  /* Privileged RW, Unprivileged None */

#define MPU_RBAR_XN (MPU_RBAR_XN_Msk)  /* Execute Never */
#define MPU_RBAR_SH (MPU_RBAR_SH_Msk)  /* Shareability */
#define MPU_RBAR_SH_LSB (MPU_RBAR_SH_Pos)
#endif
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
 * @param cls region access class, see @ref __MPU_flags for available access classes
 * @return E_OK if region was configured, otherwise error code is returned
 */
int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint8_t cls);

/** Create configuration for MPU region.
 *
 * @TODO
 */
int mpu_configure_region(uint8_t region, const void * base, uint32_t size, uint8_t flags, struct MPU_Registers * region_def);

/** Disable MPU region.
 * This function will disable use of MPU region. Address and size
 * will remain configured, but region is not marked as enabled anymore.
 * @param region ID of region being deactivated (0 - 7)
 * @return E_OK if region was deactivated
 */
int mpu_clear_region(uint8_t region);


int mpu_load(const MPU_State * state, uint8_t base, uint8_t count);

/** @} */

