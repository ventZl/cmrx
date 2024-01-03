/** @defgroup os_mpu Memory protection
 *
 * @ingroup os
 *
 * Kernel internals in support for MPU configuration.
 * 
 * Real implementation of memory protection is heavily dependant on specific
 * hardware capabilities. See @ref arch_arch for details on expected API supported by
 * architecture support layer regarding memory protection.
 */

/** @ingroup os_mpu
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

/** @} */
