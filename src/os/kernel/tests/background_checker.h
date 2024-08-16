#pragma once

#include <threads.h>

#include <stdlib.h>
#include <stdbool.h>

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

struct barrier_t;

struct checker_t {
    thrd_t thread;
    void * data;
    char * templ;
    size_t data_size;
    size_t templ_count;
    unsigned * semaphore;
    struct barrier_t * lock_barrier;
    struct barrier_t * unlock_barrier;
    bool running;
    bool result;
};

/** Type to hold barrier used to synchronize normal code and checker.
 */
struct barrier_t {
    mtx_t mutex;
    cnd_t cv;
};

enum eCheckerResult {
    OK = 0,
    DataMismatch,
    LockCountMismatch,
    InternalError
};

/** Creates atomicity checker.
 * Takes pointer to @ref data_size sized data. It then periodically checks
 * if the data is of an expected format.
 * Check is suspended if semaphore has non-zero value. This allows to emulate
 * atomic behavior.
 */
struct checker_t * checker_create(void * data, void * templ, size_t data_size, size_t templ_count, unsigned * semaphore);

/** Finish checker run.
 * @returns return value of checker run:
 * OK if all samples checked matched and every sample was checked
 * DataMismatch if any of samples was not as specified by the template
 * LockCountMismatch if amount of lock events does not match amount of samples - 1
 * InternalError if there was memory allocation error or checker was already stopped when function was called
 */
enum eCheckerResult checker_finish(struct checker_t * checker);

/** Create synchronization barrier object.
 * This object may be used to synchronize two threads. Thread which created the barrier
 * can call @ref barrier_wait to wait for someone else to unblock it.
 * @returns barrier object or NULL pointer if creation failed
 */
struct barrier_t * barrier_create();

/** Wait on barrier to be unlocked.
 * This call can be used from thread which created the barrier object only. It will
 * force the calling thread to wait until another thread unblocks it. This is used
 * to synchronize main thread and checker thread.
 */
bool barrier_wait(struct barrier_t * barrier);

/** Unblock barrier.
 * If anyone is waiting on barrier, then this will unblock it. Otherwise it does 
 * nothing.
 */
bool barrier_release(struct barrier_t * barrier);

/** Free barrier object.
 */
void barrier_free(struct barrier_t * barrier);

