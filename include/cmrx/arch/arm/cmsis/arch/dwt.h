#include <RTE_Components.h>
#include CMSIS_device_header

#ifdef __ARM_ARCH_6M__

__STATIC_FORCEINLINE int dwt_enable_cycle_counter()
{
    return 0;
}

__STATIC_FORCEINLINE uint32_t dwt_read_cycle_counter()
{
    return 0;
}

#else

#warning "Provide implementation of DWT routines"

#endif
