#include "platform.h"

#include <kernel/mpu.h>
#include <arch/mpu_priv.h>

extern char __rp2xxx_platform_custom_start;
extern char __rp2xxx_platform_custom_end;

void os_init_platform(void)
{
    // We can do this now because MPU region setup remains unchanged
    mpu_set_region(OS_MPU_REGION_UNUSED2, (const void *) &__rp2xxx_platform_custom_start, &__rp2xxx_platform_custom_end - &__rp2xxx_platform_custom_start, MPU_RX);
}
