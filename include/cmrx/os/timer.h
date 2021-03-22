#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Kernel implementation of usleep() syscall.
 *
 */
int os_usleep(unsigned microseconds);

/** Kernel implementation of setitimer() syscall.
 *
 */
int os_setitimer(unsigned microseconds);

void os_timer_init();

bool os_schedule_timer(unsigned * delay);

void os_run_timer(uint32_t microtime);

