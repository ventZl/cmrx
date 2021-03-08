#pragma once

#include <cmrx/os/runtime.h>

uint8_t os_get_current_process(void);
uint8_t os_get_current_thread(void);
uint32_t os_get_micro_time(void);
int os_sched_yield(void);
void os_start();
void systick_setup(int xms);
int os_thread_create(entrypoint_t * entrypoint, void * data);
int os_thread_join(uint8_t thread_id);
int os_thread_exit(int status);
int os_thread_stop(uint8_t thread_id);
int os_thread_continue(uint8_t thread_id);
