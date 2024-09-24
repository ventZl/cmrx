#pragma once
#include <RTE_Components.h>
#include CMSIS_device_header

#ifdef __ARM_ARCH_6M__

#define SCB_CFSR 0

#else 

#define SCB_CFSR (SCB->CFSR)

#endif

#define SCB_ICSR (SCB->ICSR)

#define SCB_ICSR_PENDSVSET (SCB_ICSR_PENDSVSET_Msk)
#define SCB_CFSR_IACCVIOL (SCB_CFSR_IACCVIOL_Msk)
#define SCB_CFSR_DACCVIOL (SCB_CFSR_DACCVIOL_Msk)
#define SCB_CFSR_MMARVALID (SCB_CFSR_MMARVALID_Msk)

#define cortex_disable_interrupts __disable_irq
#define cortex_enable_interrupts __enable_irq

