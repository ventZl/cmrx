#pragma once

#include "defines.h"
#include <cmrx/os/sysenter.h>

typedef void (*os_task_ptr_t)(void);

struct OS_process_t;

void os_start();
int os_stack_create();
int os_thread_start(int tid);
void systick_setup(int xms);
int os_thread_create(os_task_ptr_t entry, void * data);
int os_thread_init(int tid);

__SYSCALL int sched_yield();
__SYSCALL int get_tid();
