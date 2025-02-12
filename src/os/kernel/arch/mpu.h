#pragma once

/** @addtogroup arch_arch
 * @{ 
 */

#include <stdint.h>
#include <arch/mpu.h>

/** Forward declaration of structure that holds state of MPU
 * The implementor of a port shall provide definition of this 
 * structure that contains enough space to store details of one
 * memory protection unit region.
 * Kernel will allocate enough space to contain state of MPU for
 * configured MPU context size.
 */
struct MPU_Registers;

/** Start memory protection.
 * Initialize hardware memory protection unit so that following conditions are met:
 * * RAM is not executable
 * * kernel can execute all the flash and read/write all the RAM 
 * * FLASH can optionally be executable from userspace, if hardware is not capable enough
 *   to allow for fine-grained execution access.
 * Kernel must be able to continue execution past this point.
*/
void os_memory_protection_start();

/** Initialize MPU for stack of thread.
 * Performs initialization of the MPU to enable the given thread to
 * use the stack.
 * @param thread_id Thread stack has to be initialized for
 */
int mpu_init_stack(int thread_id);

/** Load MPU settings.
 * Loads MPU settings for default amount of regions from off-CPU
 * buffer. This is suitable for store-resume during task switching.
 * @param hosted_state MPU state buffer for the current host process
 * @param parent_state MPU state buffer for the parent process
 */
int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state);

/** Disable memory protection.
 * Disables memory protection unit so that no rules are enforced by the hardware.
 * The CPU state after this call should resemble MPU state after reset.
 */
void os_memory_protection_stop();

/** @} */

