#pragma once

#define SCB_CFSR			0
#define SCB_CFSR_IACCVIOL	1
#define SCB_CFSR_DACCVIOL	1
#define SCB_CFSR_MMARVALID	1
#define SCB_MMFAR			0
#define SCB_ICSR_PENDSVSET		1

extern uint32_t SCB_ICSR;
