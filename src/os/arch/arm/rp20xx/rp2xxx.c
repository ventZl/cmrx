#include "platform.h"

#include <hardware/flash.h>
#include <kernel/mpu.h>
#include <arch/mpu_priv.h>
#include <kernel/syscall.h>
#include <cmrx/sys/syscalls.h>
#include <cmrx/defines.h>
#include <cmrx/rp2xxx/rp2xxx.h>

extern char __rp2xxx_platform_custom_start;
extern char __rp2xxx_platform_custom_end;

void os_init_platform(void)
{
    // We can do this now because MPU region setup remains unchanged
    mpu_set_region(OS_MPU_REGION_UNUSED2, (const void *) &__rp2xxx_platform_custom_start, &__rp2xxx_platform_custom_end - &__rp2xxx_platform_custom_start, MPU_RX);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

int os_sys_platform_call(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    switch (arg1) {
        case RP2xxx_FLASH_ERASE:
            flash_range_erase(arg2, arg3);
            return E_OK;

        case RP2xxx_FLASH_WRITE:
            flash_range_program(arg2, (unsigned char *) arg4, arg3);
            return E_OK;

        default:
            return E_NOTAVAIL;
    }
    return E_NOTAVAIL;
}


static SYSCALL_DEFINITION struct Syscall_Entry_t nvic_syscalls[] = {
    { SYSCALL_PLATFORM, (Syscall_Handler_t) &os_rp2xxx_platform },
};

#pragma GCC diagnostic pop
