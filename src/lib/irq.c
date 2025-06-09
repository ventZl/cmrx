#include <cmrx/ipc/signal.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL int irq_enable(uint32_t irq)
{
    (void) irq;
    __SVC(SYSCALL_ENABLE_IRQ, irq);
}

__SYSCALL int irq_disable(uint32_t irq)
{
    (void) irq;
    __SVC(SYSCALL_DISABLE_IRQ, irq);
}

