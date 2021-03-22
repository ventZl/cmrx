#pragma once

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
__SYSCALL int signal(int signo, void (*sighandler)(int));

/** Send thread a signal.
 *
 * Send a signal to the thread. 
 * @param thread recipient thread id
 * @param signal signal number
 * @returns 0. Mostly.
 */
__SYSCALL int kill(int thread, int signal);
