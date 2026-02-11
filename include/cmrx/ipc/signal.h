#pragma once

#include <stdint.h>
#include <arch/sysenter.h>
#include <cmrx/api.h>

/** @defgroup api_signal Signals
 * @ingroup api
 * API for sending signals and handling incoming signals.
 *
 * @warning Not intended for new applications.
 *
 * @note This mechanism is deprecated for delivery of user-generated custom signals.
 * As signal delivery is possible inside RPC call, doing so breaches client-server
 * separation. Users needing synchronization mechanism should migrate to
 * @ref notify_object / @ref wait_for_object API instead. This provides synchronous
 * notification delivery.
 * In the future, signals will only be usable for processing of system-defined
 * signals and, such as signals to manage thread execution (stop, continue) and to
 * signal exceptional behavior.
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
__SYSCALL int CMRX_API(signal_handler)(int signo, void (*sighandler)(uint32_t));

/** Alias for @ref signal_handler
 */
__SYSCALL int CMRX_API(signal)(int signo, void (*sighandler)(uint32_t));

/** Send thread a signal.
 *
 * Send a signal to the thread. 
 * @param thread recipient thread id
 * @param signal signal number
 * @returns 0. Mostly.
 */
__SYSCALL int CMRX_API(send_signal)(int thread, uint32_t signal);

/** Alias for @ref send_signal
 */
__SYSCALL int CMRX_API(kill)(int thread, uint32_t signal);

#ifdef CMRX_VERBOSE_API_NAMES
#define kill CMRX_API(kill)
#define signal CMRX_API(signal)
#endif

/** @} */
