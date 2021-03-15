#pragma once

#include <cmrx/os/sysenter.h>

/** Register function as current thread signal handler.
 * @param signo number of signal
 * @param sighandler address of function which handles the signal
 * @returns 0. Mostly.
 */
__SYSCALL int signal(int signo, void (*sighandler)(int));

/** Send thread a signal.
 *
 * Send a signal to the thread. 
 * @param thread recipient thread id
 * @param signal signal number
 * @returns 0. Mostly.
 */
__SYSCALL int kill(int thread, int signal);
