#pragma once

void os_mutex_init(Mutex_t * mutex);
void os_mutex_lock(Mutex_t * mutex);
void os_mutex_unlock(Mutex_t * mutex);
void os_mutex_destroy(Mutex_t * mutex);
