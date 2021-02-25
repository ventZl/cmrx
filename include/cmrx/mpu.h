#pragma once

#include <stdint.h>
#include <conf/kernel.h>
#include <libopencm3/cm3/mpu.h>


#define MPU_NONE			0
#define MPU_RX				MPU_RASR_ATTR_AP_PRW_URO
#define MPU_RWX				MPU_RASR_ATTR_AP_PRW_URW
#define MPU_R				MPU_RASR_ATTR_XN | MPU_RASR_ATTR_AP_PRW_URO
#define MPU_RW				MPU_RASR_ATTR_XN | MPU_RASR_ATTR_AP_PRW_URW

#define MPU_AP_MASK			0b0111
#define MPU_EXECUTE_SHIFT	3

struct MPU_Registers {
	uint32_t _MPU_RBAR;
	uint32_t _MPU_RASR;
};

typedef struct MPU_Registers MPU_State[MPU_STATE_SIZE];

/** Enable memory protection.
 * This routine enables memory protection with standard memory setup
 * for kernel purposes. This means that any privileged code has full
 * access to memory as if no memory protection was turned on.
 * @note It is safe to call this routine in kernel context even if no
 * memory regions are set.
 */
void mpu_enable();

/** Disable memory protection.
 * This routine will disable memory protection even for unprivileged
 * code.
 */
void mpu_disable();

/** Store MPU settings.
 * Stores MPU settings for default amount of regions into off-CPU
 * buffer. This is suitable for store-resume during task switching.
 * @param state MPU state buffer
 */
int mpu_store(MPU_State * state);

/** Load MPU settings.
 * Loads MPU settings for default amount of regions from off-CPU
 * buffer. This is suitable for store-resume during task switching.
 * @param state MPU state buffer
 */
int mpu_restore(const MPU_State * state);

/** Configure and activate MPU region.
 * Activate given memory region with new base address and size.
 * For now, caller is responsible for providing base address, which is aligned
 * to size of block. If region is already activated, then its base address,
 * size and flags are being overwritten.
 * @param region ID of region being activated (0-7)
 * @param base base address of region
 * @param size size of region (256B and more)
 * @param flags region access flags
 * @return E_OK if region was configured, otherwise error code is returned
 */
int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint32_t flags);

/** Disable MPU region.
 * This function will disable use of MPU region. Address and size
 * will remain configured, but region is not marked as enabled anymore.
 * @param region ID of region being deactivated (0 - 7)
 * @return E_OK if region was deactivated
 */
int mpu_clear_region(uint8_t region);
