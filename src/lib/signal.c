#include <cmrx/ipc/signal.h>
#include <cmrx/sys/syscalls.h>
#include <cmrx/api.h>

__SYSCALL int signal(int signo, void (*sighandler)(uint32_t)) __attribute__((alias("signal_handler")));

__SYSCALL int signal_handler(int signo, void (*sighandler)(uint32_t))
{
    (void) signo;
    (void) sighandler;
	__SVC(SYSCALL_SIGNAL, signo, sighandler);
}

__SYSCALL int kill(int thread, uint32_t signal) __attribute__((alias("send_signal")));

__SYSCALL int send_signal(int thread, uint32_t signal)
{
    (void) thread;
    (void) signal;
    __SVC(SYSCALL_KILL, thread, signal);
}
