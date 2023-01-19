#pragma once
#include <RTE_Components.h>
#include CMSIS_device_header

#ifdef __ARM_ARCH_6M__

// ARM v6M doesn't have any of these
// provide some defaults which will make the code happy

#define SCB_CFSR			0
#define SCB_CFSR_IACCVIOL	1
#define SCB_CFSR_DACCVIOL	1
#define SCB_CFSR_MMARVALID	1
#define SCB_MMFAR			0

#endif

#define MPU_CTRL (MPU->CTRL)
#define MPU_RNR (MPU->RNR)
#define MPU_RASR (MPU->RASR)
#define MPU_RBAR (MPU->RBAR)

#define MPU_RASR_ENABLE (MPU_RASR_ENABLE_Msk)
#define MPU_RNR_REGION (MPU_RNR_REGION_Msk)
#define MPU_RNR_REGION_LSB (MPU_RNR_REGION_Pos)
#define MPU_RASR_SIZE (MPU_RASR_SIZE_Msk)
#define MPU_RASR_SIZE_LSB (MPU_RASR_SIZE_Pos)
#define MPU_RASR_SRD (MPU_RASR_SRD_Msk)
#define MPU_RASR_SRD_LSB (MPU_RASR_SRD_Pos)

#define MPU_RBAR_VALID (MPU_RBAR_VALID_Msk)
#define MPU_RBAR_ADDR (MPU_RBAR_ADDR_Msk)
#define MPU_RBAR_REGION (MPU_RBAR_REGION_Msk)
#define MPU_RBAR_REGION_LSB (MPU_RBAR_REGION_Pos)

#define MPU_RASR_ATTR_C (MPU_RASR_C_Msk)
#define MPU_RASR_ATTR_XN (MPU_RASR_XN_Msk)
#define MPU_RASR_ATTR_AP (MPU_RASR_AP_Msk)
#define MPU_RASR_ATTR_AP_PRW_URO (ARM_MPU_AP_URO << MPU_RASR_AP_Pos)
#define MPU_RASR_ATTR_AP_PRW_URW (ARM_MPU_AP_FULL << MPU_RASR_AP_Pos)


#define MPU_CTRL_ENABLE (MPU_CTRL_ENABLE_Msk)
#define MPU_CTRL_PRIVDEFENA (MPU_CTRL_PRIVDEFENA_Msk)

