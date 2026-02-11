#include <cmrx/ipc/mutex.h>
#include <cmrx/ipc/thread.h>
#include <cmrx/assert.h>

__SYSCALL int __futex_fast_lock(futex_t * futex, uint8_t thread_id, unsigned max_depth)
{
    (void) futex;
    (void) thread_id;
    (void) max_depth;
    /* TODO: Kernel doesn't support LDREX emulation */
    return E_NOTAVAIL;
}

__SYSCALL int __futex_fast_unlock(futex_t * futex, uint8_t thread_id)
{
    (void) futex;
    (void) thread_id;
    /* TODO: Kernel doesn't support LDREX emulation */
    return E_NOTAVAIL;
}

int futex_init(futex_t * restrict futex)
{
    futex->owner = 0xFF;
    futex->state = 0;
    return 0;
}

int futex_lock(futex_t * futex)
{
    uint8_t thread_id = get_tid();
    int success;
    do {
        success = __futex_fast_lock(futex, thread_id, 0);
        if (success != 0)
        {
            sched_yield();
        }
    } while (success != 0);
    futex->owner = thread_id;
    return 0;
}

int futex_trylock(futex_t * futex)
{
    uint8_t thread_id = get_tid();
    int success = __futex_fast_lock(futex, thread_id, 0);
    return success;
}

int futex_unlock(futex_t * futex)
{
    uint8_t thread_id = get_tid();
    int success;
    do {
        success = __futex_fast_unlock(futex, thread_id);
        ASSERT(success == 0 || futex->owner == thread_id);
    } while (success != 0);
    return success;
}

int futex_destroy(futex_t* futex)
{
    futex->state = 0;
    futex->owner = 0xFF;
}
