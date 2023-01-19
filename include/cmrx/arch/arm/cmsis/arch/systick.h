#pragma once
#include <RTE_Components.h>
#include CMSIS_device_header

__STATIC_INLINE void systick_enable()
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}
