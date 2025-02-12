#pragma once


#include <arch/sysenter.h>

/** @defgroup api_irq ARM IRQ management
 *
 * @ingroup api
 *
 * CMRX offers a tiny set of hardware-oriented services that allow
 * manipulating the status of interrupt request lines. This is provided
 * in order to give drivers access to IRQ configuration from userspace,
 * which may otherwise be denied by the hardware.
 *
 * @{
 */

/** Enable IRQ line
 *
 * This syscall will enable IRQ line identified by the IRQ number.
 * The implementation of this call is platform-specific and IRQ number
 * has hardware-specific meaning. CMRX kernel does not perform any
 * interpretation of these values.
 *
 * In general it is not an error trying to enable already enabled IRQ.
 *
 * @param irq IRQ number of interrupt line to be enabled
 * @returns E_OK if IRQ has been enabled, E_NOTAVAIL if IRQ couldn't
 * be enabled.
 */
__SYSCALL int irq_enable(uint32_t irq);

/** Disable IRQ line
 *
 * This syscall will disable IRQ line identified by the IRQ number.
 * The implementation of this call is platform-specific and IRQ number
 * has hardware-specific meaning. CMRX kernel does not perform any
 * interpretation of these values.
 *
 * In general it is not an error trying to disable already disabled IRQ.
 *
 * @param irq IRQ number of interrupt line to be disabled
 * @returns E_OK if IRQ has been disabled, E_NOTAVAIL if IRQ couldn't
 * be disabled.
 */
__SYSCALL int irq_disable(uint32_t irq);

/** @} */
