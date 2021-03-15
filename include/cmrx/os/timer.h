#pragma once

/** Kernel implementation of usleep() syscall.
 *
 */
int os_usleep(unsigned microseconds);

/** Kernel implementation of setitimer() syscall.
 *
 */
int os_setitimer(unsigned microseconds);
