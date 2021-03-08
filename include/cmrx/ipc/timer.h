#pragma once

#include <cmrx/os/sysenter.h>

__SYSCALL int usleep(unsigned microseconds);
__SYSCALL int setitimer(unsigned microseconds);
