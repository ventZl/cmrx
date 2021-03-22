#pragma once

#include <stdint.h>

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
int os_kill(uint8_t thread, uint8_t signal_id);
