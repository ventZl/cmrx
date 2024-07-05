#include "background_checker.h"

#include <memory.h>
#include <string.h>

int _checker_main(void * _d)
{
    struct checker_t * checker = (struct checker_t *) _d;
    char * buf = malloc(checker->data_size);
    memset(buf, 0, checker->data_size);

    int result = OK;
    unsigned cursor = 0;

    while (checker->running) {
        if (*checker->semaphore != 0) {
            cursor++;
            // Unlock barrier which is holding test blocked
            barrier_release(checker->lock_barrier);

            // Block itself until test unlocks the BKL
            barrier_wait(checker->unlock_barrier);
        }
        if (cursor >= checker->templ_count)
        {
            // Skip comparing further data, just release barriers so the main thread
            // doesn't get blocked by us
            result = LockCountMismatch;
            continue;
        }

        memcpy(buf, checker->data, checker->data_size);

        if (memcmp(buf, &checker->templ[cursor * checker->data_size], checker->data_size) != 0) {
            result = DataMismatch;
        }

    }

    if (cursor != checker->templ_count - 1)
    {
        result = LockCountMismatch;
    }

    return result;
}


struct checker_t * checker_create(void * data, void * templ, size_t data_size, size_t templ_count, unsigned * semaphore) 
{
    if (data == NULL || templ == NULL || semaphore == NULL)
    {
        return NULL;
    }

    struct checker_t * checker = malloc(sizeof(struct checker_t));
    
    if (checker == NULL) {
        return checker;
    }

    checker->data = data;
    checker->templ = templ;
    checker->data_size = data_size;
    checker->templ_count = templ_count;
    checker->semaphore = semaphore;
    checker->running = true;
    checker->result = true;
    checker->lock_barrier = barrier_create();
    checker->unlock_barrier = barrier_create();

    int rv = thrd_create(&checker->thread, _checker_main, checker);

    if (rv != thrd_success)
    {
        free(checker);
        return NULL;
    }

    return checker;
}

enum eCheckerResult checker_finish(struct checker_t *checker)
{
    if (checker == NULL)
    {
        return InternalError;
    }

    if (checker->running == false)
    {
        thrd_join(checker->thread, NULL);
        free(checker);
        return InternalError;
    }

    checker->running = false;

    int rv = 0;

    thrd_join(checker->thread, &rv);

    barrier_free(checker->lock_barrier);
    barrier_free(checker->unlock_barrier);
    free(checker);

    return rv;
}

struct barrier_t * barrier_create() 
{
    struct barrier_t * barrier = malloc(sizeof(struct barrier_t));
    
    if (barrier == NULL) {
        return barrier;
    }

    int rv = mtx_init(&barrier->mutex, mtx_plain);

    if (rv != thrd_success)
    {
        free(barrier);
        return NULL;
    }
    
    rv = cnd_init(&barrier->cv);

    if (rv != thrd_success)
    {
        mtx_destroy(&barrier->mutex);
        free(barrier);
        return NULL;
    }

    mtx_lock(&barrier->mutex);

    return barrier;
}

bool barrier_wait(struct barrier_t * barrier)
{
    if (barrier == NULL)
    {
        return false;
    }

    cnd_wait(&barrier->cv, &barrier->mutex);

    return true;
}

bool barrier_release(struct barrier_t * barrier)
{
    if (barrier == NULL)
    {
        return false;
    }

    cnd_signal(&barrier->cv);

    return true;
}

void barrier_free(struct barrier_t * barrier)
{
    if (barrier == NULL)
    {
        return;
    }

    mtx_unlock(&barrier->mutex);
    mtx_destroy(&barrier->mutex);
    cnd_destroy(&barrier->cv);

    free(barrier);
}
