#pragma once

#include "defines.h"

typedef void (*os_task_ptr_t)(void);

void os_start();
int os_stack_create();
int os_task_start(int tid);
void systick_setup(int xms);
int os_task_create(const struct OS_task_t * task);
int os_task_init(int tid);

