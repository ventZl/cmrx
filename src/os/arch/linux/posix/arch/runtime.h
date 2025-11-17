#pragma once

#include <ucontext.h>
#include <threads.h>
#include <stdatomic.h>
#include <pthread.h>

#define os_init_core(x)

struct OS_thread_t;

void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data);
void os_init_arch(void);

struct Arch_State_t {
    int block_pipe[2];  // [read_fd, write_fd]
    volatile atomic_int is_suspended;
    thrd_t sched_thread;
    pthread_t sched_thread_id;
};

