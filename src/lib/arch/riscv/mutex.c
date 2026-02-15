/*
 * RISC-V mutex/futex implementation for CMRX.
 *
 * Note: This is a minimal single-core implementation using interrupt
 * disabling for mutual exclusion. A proper multi-core implementation
 * would require RISC-V AMO (atomic memory operation) instructions.
 */

#include <cmrx/ipc/mutex.h>
#include <cmrx/ipc/thread.h>
#include <cmrx/ipc/notify.h>
#include <cmrx/sys/notify.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>
#include <cmrx/arch/riscv/hal.h>

static inline uint32_t irq_save(void)
{
    uint32_t mstatus = cmrx_riscv_csr_read_mstatus();
    cmrx_riscv_irq_disable();
    return mstatus;
}

static inline void irq_restore(uint32_t mstatus)
{
    cmrx_riscv_csr_write_mstatus(mstatus);
}

int __futex_fast_lock(futex_t *futex, uint8_t thread_id, unsigned max_depth)
{
    uint32_t saved = irq_save();
    int success = FUTEX_FAILURE;
    (void)max_depth;

    unsigned state = futex->state;
    if (state == 0) {
        state++;
        futex->state = state;
        futex->owner = thread_id;
        success = FUTEX_SUCCESS;
    }

    irq_restore(saved);
    return success;
}

int __futex_fast_unlock(futex_t *futex, uint8_t thread_id)
{
    uint32_t saved = irq_save();
    int success = FUTEX_FAILURE;

    unsigned state = futex->state;
    if (state > 0) {
        ASSERT(futex->owner == thread_id);
        if (futex->owner == thread_id) {
            state--;
            futex->state = state;
            success = FUTEX_SUCCESS;
        }
    }

    irq_restore(saved);
    return success;
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
