/** @defgroup api_signal Signals
 *
 * @ingroup api
 *
 * Threads can send signals asynchronously to each other. Kernel provides
 * mechanism to register signal handler, which gets executed whenever thread
 * receives a signal. There are two kinds of signals:
 *
 * * catchable - kernel supports sending of 32 distinct catchable signals. Kernel
 * doesn't interpret any of them in any way. If such signal is sent to the thread
 * then thread is simply notified of its arrival.
 * * non-catchable - these signals are mostly system-defined and thread is not
 * able to catch nor react to them. These include stopping and resuming thread,
 * killing it and signalling memory protection violation error.
 *
 * If thread doesn't register any signal handler, then signal arrival is effectively
 * a no-op for given thread. In any case, arrival of signal will wake thread up,
 * if it is stopped.
 */

/** @ingroup api_signal
 * @{
 */
#pragma once

#include <stdint.h>
#include <cmrx/os/sysenter.h>

#define SIGALRM					0

#define SIGKILL					32
#define SIGSTOP					33
#define SIGCONT					34
#define SIGSEGV					35

/** Register function as current thread signal handler.
 * @param signo number of signal
 * @param sighandler address of function which handles the signal
 * @returns 0. Mostly.
 */
__SYSCALL int signal(int signo, void (*sighandler)(uint32_t));

/** Send thread a signal.
 *
 * Send a signal to the thread. 
 * @param thread recipient thread id
 * @param signal signal number
 * @returns 0. Mostly.
 */
__SYSCALL int kill(int thread, uint32_t signal);

/** @} */
