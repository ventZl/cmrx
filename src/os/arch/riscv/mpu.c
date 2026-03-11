/*
 * CMRX RISC-V Memory Protection stubs.
 *
 * RISC-V uses PMP (Physical Memory Protection) instead of MPU.
 * This file provides stub implementations to allow the kernel to build
 * without actual memory protection.
 *
 * Memory protection for RISC-V is NOT implemented in this port.
 * These stubs allow the kernel to link and run without protection.
 */

#include <kernel/mpu.h>
#include <kernel/sched.h>
#include <cmrx/defines.h>
#include <arch/mpu.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Restore MPU state.
 * Stub - does nothing since PMP is not implemented.
 */
int mpu_restore(const MPU_State *hosted_state, const MPU_State *parent_state)
{
    (void)hosted_state;
    (void)parent_state;
    return E_OK;
}

/*
 * Initialize stack protection for a thread.
 * Stub - does nothing since PMP is not implemented.
 */
int mpu_init_stack(int thread_id)
{
    (void)thread_id;
    return E_OK;
}

/*
 * Start memory protection.
 * Stub - does nothing since PMP is not implemented.
 */
void os_memory_protection_start(void)
{
    /* PMP not implemented */
}

/*
 * Stop memory protection.
 * Stub - does nothing since PMP is not implemented.
 */
void os_memory_protection_stop(void)
{
    /* PMP not implemented */
}

/*
 * Set an MPU region.
 * Stub - does nothing since PMP is not implemented.
 */
int mpu_set_region(uint8_t region, const void *base, uint32_t size, uint8_t cls)
{
    (void)region;
    (void)base;
    (void)size;
    (void)cls;
    return E_OK;
}

/*
 * Configure an MPU region.
 * Stub - does nothing since PMP is not implemented.
 */
int mpu_configure_region(uint8_t region, const void *base, uint32_t size,
                         uint8_t cls, uint32_t *RBAR, uint32_t *RASR)
{
    (void)region;
    (void)base;
    (void)size;
    (void)cls;

    if (RBAR) *RBAR = 0;
    if (RASR) *RASR = 0;

    return E_OK;
}

/*
 * Clear an MPU region.
 * Stub - does nothing since PMP is not implemented.
 */
int mpu_clear_region(uint8_t region)
{
    (void)region;
    return E_OK;
}

/*
 * Check if an address is within MPU bounds.
 * Stub - always returns false since PMP is not implemented.
 */
bool mpu_check_bounds(const MPU_State *state, uint8_t region, uint32_t *address)
{
    (void)state;
    (void)region;
    (void)address;
    return false;
}
