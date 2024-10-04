#pragma once

/** @defgroup api_isr Interrupt Service Routines
 * @ingroup api
 * Kernel services accessible from interrupt service handlers.
 *
 * Kernel interface is not accessible from the realm of interrupt service routines.
 * Normally, interrupt service routines can't call the kernel syscalls. This behavior is
 * by design. The idea is to do as little of work in the context of the ISR handler and 
 * then either pass the CPU to the interrupted thread, or wake up some specific thread to
 * finish the work.
 * Calls present in this section are not system calls, can only be called from interrupt
 * handler context.
 * @{
 */

#include <cmrx/defines.h>
#include <stdint.h>

/** Send signal from ISR context.
 * This routine is an equivalent of \ref kill() syscall, which
 * is usable from interrupt service routine context.
 * @param thread_id thread, which should receive the signal
 * @param signal signal to be sent
 */
void isr_kill(Thread_t thread_id, uint32_t signal);

/** Notify object from ISR context.
 * This routine is an equivalent of \ref notify_object() syscall
 * which is usable from interrupt service routine context.
 * @param object object to be notified
 */
void isr_notify_object(const void * object);


/** @} */
