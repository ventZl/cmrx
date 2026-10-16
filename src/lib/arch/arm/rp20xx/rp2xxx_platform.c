#include <cmrx/sys/syscalls.h>
#include <arch/sysenter.h>
#include <stdint.h>

__SYSCALL int rp2xxx_platform_call(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    __SVC(SYSCALL_PLATFORM, arg1, arg2, arg3, arg4);
}

int flash_erase(uint32_t * base, uint32_t length)
{
    rp2xxx_platform_call(1, (uint32_t) base, length, 0);
}

int flash_write(uint32_t * base, uint32_t length, const uint32_t * source)
{
    rp2xxx_platform_call(2, (uint32_t) base, length, (uint32_t) source);
}



