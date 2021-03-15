#pragma once

enum Signals {
	SIGALARM
};

/** Kernel implementation of signal syscall.
 *
 */
int os_signal(int signo, void (*sighandler)(int));

/** Kernel implementation of kill syscall.
 *
 */
int os_kill(int thread, int signal);
