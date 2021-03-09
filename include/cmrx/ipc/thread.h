#pragma once

#include <cmrx/os/sysenter.h>

__SYSCALL int get_tid();
__SYSCALL int sched_yield();
__SYSCALL int thread_create(int *entrypoint(void *), void * data, uint8_t priority);
__SYSCALL int thread_join(int thread, int * status);
__SYSCALL int thread_exit(int status);

__SYSCALL void usleep(uint32_t microseconds);
__SYSCALL void setitimer(uint32_t microseconds);
