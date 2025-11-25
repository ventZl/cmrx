#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"

typedef void (IRQ_Handler_t)();

__attribute__((weak)) void IRQ_0() {};
__attribute__((weak)) void IRQ_1() {};
__attribute__((weak)) void IRQ_2() {};
__attribute__((weak)) void IRQ_3() {};
__attribute__((weak)) void IRQ_4() {};
__attribute__((weak)) void IRQ_5() {};
__attribute__((weak)) void IRQ_6() {};
__attribute__((weak)) void IRQ_7() {};
__attribute__((weak)) void IRQ_8() {};
__attribute__((weak)) void IRQ_9() {};
__attribute__((weak)) void IRQ_10() {};
__attribute__((weak)) void IRQ_11() {};
__attribute__((weak)) void IRQ_12() {};
__attribute__((weak)) void IRQ_13() {};
extern __attribute__((weak)) void IRQ_14() {};
extern __attribute__((weak)) void IRQ_15() {};

IRQ_Handler_t * irq_table[64] = {
    IRQ_0,
    IRQ_1,
    IRQ_2,
    IRQ_3,
    IRQ_4,
    IRQ_5,
    IRQ_6,
    IRQ_7,
    IRQ_8,
    IRQ_9,
    IRQ_10,
    IRQ_11,
    IRQ_12,
    IRQ_13,
    IRQ_14,
    IRQ_15,
};

volatile uint32_t interrupt_flags = 0;

void generate_interrupt(unsigned irq_no)
{
    interrupt_flags |= 1 << irq_no;
    kill(getpid(), SIGUSR2);
}

void irq_handler(int signo)
{
    assert(signo == SIGUSR2);
    assert(current_thread_id != -1);

    for (int q = 0; q < 16; ++q)
    {
        if (interrupt_flags & (1 << q))
        {
            irq_table[q]();
        }
        interrupt_flags &= ~(1 << q);
    }
}

void irq_init()
{
    /* SIGUSR2 has the highest priority and it will preempt
     * SIGALRM, SIGURG but not SIGUSR1, which will "disable interrupts"
     * during its exection.
     */
    struct sigaction sigusr2_action = { 0 };
//    sigusr2_action.sa_flags = SA_NODEFER;
    sigusr2_action.sa_handler = &irq_handler;
    sigemptyset(&sigusr2_action.sa_mask);

    sigaction(SIGUSR2, &sigusr2_action, NULL);
}
