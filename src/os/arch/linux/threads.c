#include <threads.h>
#include <pthread.h>

typedef void * (*pthread_start_t)(void *);

int thrd_create(thrd_t * thr, thrd_start_t func, void * arg)
{
    pthread_attr_t attr;
    pthread_start_t pthread_func = (pthread_start_t) func;
    pthread_attr_init(&attr);
    return pthread_create(thr, &attr, pthread_func, arg);
}
int mtx_init(mtx_t * mtx, int type)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int ptype = PTHREAD_MUTEX_NORMAL;

    switch (type) {
        case mtx_recursive:
            ptype = PTHREAD_MUTEX_RECURSIVE;
            break;

        default:
            break;
    }
    pthread_mutexattr_settype(&attr, ptype);
    return pthread_mutex_init(mtx, &attr);
}

int mtx_lock(mtx_t * mtx)
{
    return pthread_mutex_lock(mtx);
}

int mtx_unlock(mtx_t * mtx)
{
    return pthread_mutex_unlock(mtx);
}

void mtx_destroy(mtx_t * mtx)
{
    (void) pthread_mutex_destroy(mtx);
}