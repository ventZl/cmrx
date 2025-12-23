#pragma once

#include <stdint.h>

#define __STATIC_FORCEINLINE static inline
#define __enable_irq()
#define __disable_irq()
#define __set_CONTROL(x)
#define __ISB()
#define __DSB()

extern volatile uint32_t __PSP;

#define __set_PSP(x) __PSP = (x)
#define __get_PSP(x) __PSP

void NVIC_EnableIRQ(unsigned irq)
{
    (void) irq;
}

void NVIC_DisableIRQ(unsigned irq)
{
    (void) irq;
}

void __WFI()
{
}

typedef struct {
    volatile uint32_t AIRCR;
#if defined __ARM_ARCH_7M__ || defined __ARM_ARCH_8M__
    volatile uint32_t CFSR;
#endif
    volatile uint32_t ICSR;
} SCB_Type;

extern SCB_Type * SCB;

#define SCB_AIRCR_VECTKEY_Pos              16U
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)

#define SCB_AIRCR_VECTKEYSTAT_Pos          16U
#define SCB_AIRCR_VECTKEYSTAT_Msk          (0xFFFFUL << SCB_AIRCR_VECTKEYSTAT_Pos)

#define SCB_AIRCR_ENDIANNESS_Pos           15U
#define SCB_AIRCR_ENDIANNESS_Msk           (1UL << SCB_AIRCR_ENDIANNESS_Pos)

#define SCB_AIRCR_SYSRESETREQ_Pos           2U
#define SCB_AIRCR_SYSRESETREQ_Msk          (1UL << SCB_AIRCR_SYSRESETREQ_Pos)

#define SCB_AIRCR_VECTCLRACTIVE_Pos         1U
#define SCB_AIRCR_VECTCLRACTIVE_Msk        (1UL << SCB_AIRCR_VECTCLRACTIVE_Pos)

#define SCB_ICSR_PENDSVSET_Pos             28U
#define SCB_ICSR_PENDSVSET_Msk             (1UL << SCB_ICSR_PENDSVSET_Pos)

#define SCB_ICSR_PENDSVCLR_Pos             27U
#define SCB_ICSR_PENDSVCLR_Msk             (1UL << SCB_ICSR_PENDSVCLR_Pos)

#if defined __ARM_ARCH_7M__ || defined __ARM_ARCH_8M__

#define SCB_CFSR_IACCVIOL_Pos              (SCB_CFSR_MEMFAULTSR_Pos + 0U)
#define SCB_CFSR_IACCVIOL_Msk              (1UL)

#define SCB_CFSR_DACCVIOL_Pos              (SCB_CFSR_MEMFAULTSR_Pos + 1U)
#define SCB_CFSR_DACCVIOL_Msk              (1UL << SCB_CFSR_DACCVIOL_Pos)

#define SCB_CFSR_MMARVALID_Pos             (SCB_CFSR_MEMFAULTSR_Pos + 7U)
#define SCB_CFSR_MMARVALID_Msk             (1UL << SCB_CFSR_MMARVALID_Pos)

#define SCB_CFSR_MEMFAULTSR_Pos             0U
#define SCB_CFSR_MEMFAULTSR_Msk            (0xFFUL)

#endif

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t TYPE;
    volatile uint32_t RNR;
    volatile uint32_t RBAR;
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    volatile uint32_t RLAR;   /* ARMv8M: Region Limit Address Register */
    volatile uint32_t MAIR0;  /* ARMv8M: Memory Attribute Indirection Register 0 */
    volatile uint32_t MAIR1;  /* ARMv8M: Memory Attribute Indirection Register 1 */
#else
    volatile uint32_t RASR;   /* ARMv6M/ARMv7M: Region Attribute and Size Register */
#endif
} MPU_Type;

extern MPU_Type * MPU;


#define MPU_TYPE_IREGION_Pos               16U
#define MPU_TYPE_IREGION_Msk               (0xFFUL << MPU_TYPE_IREGION_Pos)

#define MPU_TYPE_DREGION_Pos                8U
#define MPU_TYPE_DREGION_Msk               (0xFFUL << MPU_TYPE_DREGION_Pos)

#define MPU_TYPE_SEPARATE_Pos               0U
#define MPU_TYPE_SEPARATE_Msk              (1UL


#define MPU_CTRL_PRIVDEFENA_Pos             2U
#define MPU_CTRL_PRIVDEFENA_Msk            (1UL << MPU_CTRL_PRIVDEFENA_Pos)

#define MPU_CTRL_HFNMIENA_Pos               1U
#define MPU_CTRL_HFNMIENA_Msk              (1UL << MPU_CTRL_HFNMIENA_Pos)

#define MPU_CTRL_ENABLE_Pos                 0U
#define MPU_CTRL_ENABLE_Msk                (1UL)


#define MPU_RNR_REGION_Pos                  0U
#define MPU_RNR_REGION_Msk                 (0xFFUL)


#define MPU_RBAR_ADDR_Pos                   8U
#define MPU_RBAR_ADDR_Msk                  (0xFFFFFFUL << MPU_RBAR_ADDR_Pos)

#define MPU_RBAR_VALID_Pos                  4U
#define MPU_RBAR_VALID_Msk                 (1UL << MPU_RBAR_VALID_Pos)

#define MPU_RBAR_REGION_Pos                 0U
#define MPU_RBAR_REGION_Msk                (0xFUL)


#define MPU_RASR_ATTRS_Pos                 16U
#define MPU_RASR_ATTRS_Msk                 (0xFFFFUL << MPU_RASR_ATTRS_Pos)

#define MPU_RASR_XN_Pos                    28U
#define MPU_RASR_XN_Msk                    (1UL << MPU_RASR_XN_Pos)

#define MPU_RASR_AP_Pos                    24U
#define MPU_RASR_AP_Msk                    (0x7UL << MPU_RASR_AP_Pos)

#define MPU_RASR_TEX_Pos                   19U
#define MPU_RASR_TEX_Msk                   (0x7UL << MPU_RASR_TEX_Pos)

#define MPU_RASR_S_Pos                     18U
#define MPU_RASR_S_Msk                     (1UL << MPU_RASR_S_Pos)

#define MPU_RASR_C_Pos                     17U
#define MPU_RASR_C_Msk                     (1UL << MPU_RASR_C_Pos)

#define MPU_RASR_B_Pos                     16U
#define MPU_RASR_B_Msk                     (1UL << MPU_RASR_B_Pos)

#define MPU_RASR_SRD_Pos                    8U
#define MPU_RASR_SRD_Msk                   (0xFFUL << MPU_RASR_SRD_Pos)

#define MPU_RASR_SIZE_Pos                   1U
#define MPU_RASR_SIZE_Msk                  (0x1FUL << MPU_RASR_SIZE_Pos)

#define MPU_RASR_ENABLE_Pos                 0U
#define MPU_RASR_ENABLE_Msk                (1UL)

#define ARM_MPU_AP_NONE 0U
#define ARM_MPU_AP_PRIV 1U
#define ARM_MPU_AP_URO  2U
#define ARM_MPU_AP_FULL 3U
#define ARM_MPU_AP_PRO  5U
#define ARM_MPU_AP_RO   6U

/* ARMv8M MPU RLAR register fields */
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)

#define MPU_RLAR_LIMIT_Pos                  5U
#define MPU_RLAR_LIMIT_Msk                 (0x7FFFFFFUL << MPU_RLAR_LIMIT_Pos)

#define MPU_RLAR_AttrIndx_Pos               1U
#define MPU_RLAR_AttrIndx_Msk              (0x7UL << MPU_RLAR_AttrIndx_Pos)

#define MPU_RLAR_EN_Pos                     0U
#define MPU_RLAR_EN_Msk                    (1UL)

/* ARMv8M MPU RBAR additional fields (beyond base address) */
#define MPU_RBAR_XN_Pos                     0U
#define MPU_RBAR_XN_Msk                    (1UL)

#define MPU_RBAR_AP_Pos                     1U
#define MPU_RBAR_AP_Msk                    (0x3UL << MPU_RBAR_AP_Pos)

#define MPU_RBAR_SH_Pos                     3U
#define MPU_RBAR_SH_Msk                    (0x3UL << MPU_RBAR_SH_Pos)

#undef MPU_RBAR_ADDR_Msk
#define MPU_RBAR_ADDR_Pos                   5U
#define MPU_RBAR_ADDR_Msk                  (0x7FFFFFFUL << MPU_RBAR_ADDR_Pos)

#endif

#define EXC_RETURN_THREAD_PSP 0xFFFFFFFE

