/** @defgroup os_mpu Memory protection
 *
 * @ingroup os
 *
 * Kernel internals in support for MPU configuration.
 * @{
 */
#pragma once

#include <stdint.h>
#include <conf/kernel.h>

/** MPU region access rights 
 */
enum MPU_Flags {
	/// Region cannot be accesses. Any attempt to access addresses from this region will result in hard fault
	MPU_NONE,
	/// Region can be read and executed. Attempt to write will result in hard fault
	MPU_RX,
	/// Region can be read, written and executed. No attempt to access region can result in hard fault
	MPU_RWX,
	/// Region can be read. Attempt to write into region, or execute out of it will result in hard fault
	MPU_R,
	/// Region can be read and written but cannot be executed. Attempt to execute code will result in hard fault
	MPU_RW
};

#define MPU_AP_MASK			0b0111
#define MPU_EXECUTE_SHIFT	3

/** @defgroup mpu_region_names MPU region names 
 * @{
 */
/// Region for initialized readable/writable data 
#define OS_MPU_REGION_DATA			0
/// Region for uninitialized readable/writable data
#define OS_MPU_REGION_BSS			1
/// Region covering memory-mapped IO devices
#define OS_MPU_REGION_MMIO			2
/// Region containing shared/sharable resources
#define OS_MPU_REGION_MMIO2 		3
/// Currently unused region (reserved)
#define OS_MPU_REGION_SHARED		4
/// Currently unused region (reserved)
#define OS_MPU_REGION_UNUSED2		5
/// Region covering thread's stack
#define OS_MPU_REGION_STACK			6
/// Region covering executable RAM (?)
#define OS_MPU_REGION_EXECUTABLE	7
/** @} */

/** Convenience structure which groups RBAR and RASR register 
 * for one MPU region together
 */
struct MPU_Registers {
	uint32_t _MPU_RBAR;
	uint32_t _MPU_RASR;
};

/** Type handling MPU state as remembered by CMRX thread switcher
 */
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
int mpu_store(MPU_State * hosted_state, MPU_State * parent_state);

/** Load MPU settings.
 * Loads MPU settings for default amount of regions from off-CPU
 * buffer. This is suitable for store-resume during task switching.
 * @param state MPU state buffer
 */
int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state);

int mpu_load(const MPU_State * state, uint8_t base, uint8_t count);

/** Initialize MPU for stack of thread.
 * Performs initialization of the MPU to enable the given thread to
 * use the stack.
 * @param thread_id Thread stack has to be initialized for
 */
int mpu_init_stack(int thread_id);

/** @} */
