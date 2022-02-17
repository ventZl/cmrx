#pragma once

#include <libopencm3/cm3/scb.h>

#ifdef __ARM_ARCH_6M__

#define SCB_CFSR			0
#define SCB_CFSR_IACCVIOL	1
#define SCB_CFSR_DACCVIOL	1
#define SCB_CFSR_MMARVALID	1
#define SCB_MMFAR			0

#endif

