#pragma once

#include <ucontext.h>

#define os_init_core(x)

struct OS_thread_t;

void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data);
void os_init_arch(void);

struct Arch_State_t {
    ucontext_t thread_context;
};

