#include <cmrx/sys/ipi.h>
#include <arch/corelocal.h>
#include <conf/kernel.h>

#include <RTE_Components.h>
#include CMSIS_device_header

#include <stdint.h>
#include <stdbool.h>

static volatile uint8_t sync_requested = 0;
static volatile uint8_t sync_cores[OS_NUM_CORES] = {0};

void os_ipi_sync_request()
{
    for (unsigned q = 0; q < OS_NUM_CORES; ++q)
    {
        sync_cores[q] = 0;
    }
    sync_requested = 1;
    __DSB();
    sync_cores[coreid()] = 1;
    __disable_irq();
    __SEV();
    bool synced = true;
    do {
        synced = true;
        for (unsigned q = 0; q < OS_NUM_CORES; ++q)
        {
            if (sync_cores[q] == 0)
            {
                synced = false;
            }
        }
    } while (!synced);
}

void os_ipi_sync_release()
{
    sync_requested = 0;
    __DSB();
    __SEV();
    __enable_irq();
}
void os_ipi_sync_probe()
{
    if (sync_requested)
    {
        __disable_irq();
        sync_cores[coreid()] = 1;
        do {
            __WFE();
        } while (sync_requested == 1);
        sync_cores[coreid()] = 0;

        __enable_irq();
    }
}

