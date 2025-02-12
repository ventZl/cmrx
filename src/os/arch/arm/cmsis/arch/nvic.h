#pragma once

/** @defgroup arch_arm_nvic Memory protection
 *
 * @ingroup arch_arm
 *
 * Exposure of Cortex-M NVIC to the userspace.
 * @{
 */


#include <stdint.h>

/** Implementation of irq_enable syscall for ARM Cortex-M
 * See @ref irq_enable for more details on arguments.*/
int os_nvic_enable(uint32_t irq);

/** Implementation of irq_disable syscall for ARM Cortex-M
 * See @ref irq_disable for more details on arguments.*/
int os_nvic_disable(uint32_t irq);

/** @} */
