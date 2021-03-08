#pragma once

#include <cmrx/os/sysenter.h>

__SYSCALL int signal(int signo, void (*sighandler)(int));
__SYSCALL int kill(int thread, int signal);
