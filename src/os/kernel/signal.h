/** @defgroup os_signal Signal handling
 * @ingroup os
 * Kernel implementation of signal propagation mechanism.
 */

/** @ingroup os_signal
 * @{
 */
#pragma once

#include <stdint.h>

enum Signals {
	SIGALARM
};

struct OS_thread_t;

/** Internal implementation of signal delivery into thread context.
 *
 * As this routine is only designed to be called from pend_sv handler,
 * it has strict requirements on state of thread. It must be in state
 * just before being resumed.
 * @param thread thread which gets signal handler injected
 * @param signals mask of signals being delivered
 */
void os_deliver_signal(struct OS_thread_t * thread, uint32_t signals);

/** Kernel implementation of signal syscall.
 *
 */
int os_signal(int signo, void (*sighandler)(int));

/** Kernel implementation of kill syscall.
 *
 */
int os_kill(uint8_t thread, uint8_t signal_id);

/** @} */
