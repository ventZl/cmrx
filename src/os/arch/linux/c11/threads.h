#pragma once

#include <pthread.h>

typedef pthread_t thrd_t;
typedef int (*thrd_start_t)(void *);

typedef pthread_mutex_t mtx_t;

#define mtx_plain 0
#define mtx_recursive 1
#define mtx_timed 2

#define thrd_success 0
#define thrd_error 1

int thrd_create(thrd_t * thr, thrd_start_t func, void * arg);
int mtx_init(mtx_t * mtx, int type);
int mtx_lock(mtx_t * mtx);
int mtx_unlock(mtx_t * mtx);
void mtx_destroy(mtx_t * mtx);