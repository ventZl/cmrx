#pragma once

enum Signals {
	SIGALARM
};

int os_signal(int signo, void (*sighandler)(int));
int os_kill(int thread, int signal);
