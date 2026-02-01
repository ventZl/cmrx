/*
 * RISC-V mutex/futex implementation for CMRX.
 *
 * Uses LR.W/SC.W (load-reserved/store-conditional) from the RISC-V A
 * extension for atomic fast-path lock and unlock.
 *
 * futex_t must be placed at a 4-byte aligned address for LR.W/SC.W to
 * operate correctly.
 */

#if defined(__riscv_atomic) || defined(__riscv_a)

#include <cmrx/ipc/mutex.h>
#include <cmrx/ipc/thread.h>
#include <cmrx/ipc/notify.h>
#include <cmrx/assert.h>
#include <cmrx/defines.h>

/** Lock futex.
 * Perform atomic futex lock using LR.W/SC.W. It is only possible to lock an
 * unlocked mutex. Locking may fail spuriously due to hardware specifics of
 * load-reserved/store-conditional. Recursive mutexes are not supported.
 * @param futex futex to be locked
 * @param thread_id identification of calling thread
 * @param max_depth unused
 * @returns 0 if futex lock was successful, 1 if locking failed for whatever reason
 */
int __futex_fast_lock(futex_t *futex, uint8_t thread_id, unsigned max_depth)
{
    uint32_t loaded, stored, new_val;
    (void)max_depth;

    uint32_t *word = (uint32_t *)(uintptr_t)futex;

    __asm__ volatile ("lr.w %0, (%1)"
                      : "=r"(loaded) : "r"(word) : "memory");

    if ((loaded & 0xFFu) != 0) {
        return FUTEX_FAILURE;
    }

    new_val = (loaded & ~0xFFFFu) | ((uint32_t)thread_id << 8) | 1u;

    __asm__ volatile ("sc.w %0, %2, (%1)"
                      : "=r"(stored) : "r"(word), "r"(new_val) : "memory");

    return (stored == 0) ? FUTEX_SUCCESS : FUTEX_FAILURE;
}

/** Unlock futex.
 * This function performs fast unlock of futex if that is possible.
 * If mutex is not locked, this action will fail. It is an undefined
 * behavior to unlock mutex locked by another thread.
 * @param futex Futex to be unlocked
 * @param thread_id Numeric identification of futex owner
 * @returns 0 if futex unlock was successful, 1 if unlocking failed for
 * whatever reason.
 */
int __futex_fast_unlock(futex_t *futex, uint8_t thread_id)
{
    uint32_t loaded, stored, new_val;

    uint32_t *word = (uint32_t *)(uintptr_t)futex;

    __asm__ volatile ("lr.w %0, (%1)"
                      : "=r"(loaded) : "r"(word) : "memory");

    uint8_t state = (uint8_t)(loaded & 0xFFu);
    if (state == 0) {
        return FUTEX_FAILURE;
    }

    ASSERT(((loaded >> 8) & 0xFFu) == thread_id);
    if (((loaded >> 8) & 0xFFu) != thread_id) {
        return FUTEX_FAILURE;
    }

    new_val = (loaded & ~0xFFu) | (state - 1u);

    __asm__ volatile ("sc.w %0, %2, (%1)"
                      : "=r"(stored) : "r"(word), "r"(new_val) : "memory");

    return (stored == 0) ? FUTEX_SUCCESS : FUTEX_FAILURE;
}

int futex_init(futex_t *restrict futex)
{
    ASSERT(futex != NULL);
    futex->owner = 0xFF;
    futex->state = 0;
    return 0;
}

int futex_lock(futex_t *futex)
{
    ASSERT(futex != NULL);
    uint8_t thread_id = get_tid();
    int success;
    do {
        success = __futex_fast_lock(futex, thread_id, 0);
        if (success != FUTEX_SUCCESS) {
            wait_for_object_value(&futex->state, 0, 0,
                                  NOTIFY_PRIORITY_INHERIT(futex->owner));
        }
    } while (success != FUTEX_SUCCESS);
    return 0;
}

int futex_trylock(futex_t *futex)
{
    ASSERT(futex != NULL);
    uint8_t thread_id = get_tid();
    return __futex_fast_lock(futex, thread_id, 0);
}

int futex_unlock(futex_t *futex)
{
    ASSERT(futex != NULL);
    uint8_t thread_id = get_tid();
    int success;
    do {
        success = __futex_fast_unlock(futex, thread_id);
        if (success == FUTEX_SUCCESS) {
            notify_object2(&futex->state, NOTIFY_PRIORITY_DROP);
        }
    } while (success != FUTEX_SUCCESS);
    return 0;
}

int futex_destroy(futex_t *futex)
{
    ASSERT(futex != NULL);
    futex->state = 0;
    futex->owner = 0xFF;
    return 0;
}

#else

#error "RISC-V A extension (atomics) is required for the LR.W/SC.W-based mutex. \
        Target -march= does not include 'a'. Either enable the A extension or \
        provide a non-atomic fallback implementation."

#endif