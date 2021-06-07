#pragma once

/** @defgroup api_isr Interrupt Service Routines
 * 
 * @ingroup api
 *
 * Often, it is necessary to notify thread of events from interrupt service
 * routine. It is not possible to call operating system services from interrupt
 * service routines, as ISRs run at higher priority than CMRX kernel.
 * CMRX offers way how to send a signal to thread (potentially waking it up) from
 * ISR safely.
 */

/** @ingroup api_isr
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

/** @} */
