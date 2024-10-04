#include <RTE_Components.h>
#include CMSIS_device_header

#include <kernel/syscall.h>
#include <cmrx/defines.h>

int os_nvic_enable(uint32_t irq)
{
    NVIC_EnableIRQ(irq);
    return E_OK;
}

int os_nvic_disable(uint32_t irq)
{
    NVIC_DisableIRQ(irq);
    return E_OK;
}

