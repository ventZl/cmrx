/** @ingroup api_signal
 * @{
 */
#include <cmrx/ipc/signal.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL int irq_enable(uint32_t irq)
{
    (void) irq;
    __SVC(SYSCALL_ENABLE_IRQ);
}

__SYSCALL int irq_disable(uint32_t irq)
{
    (void) irq;
    __SVC(SYSCALL_DISABLE_IRQ);
}

/** @} */

