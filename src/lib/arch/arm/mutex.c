/** @ingroup api_mutex
 * @{
 */

#include <RTE_Components.h>
#include CMSIS_device_header
#include <cmrx/ipc/mutex.h>
#include <cmrx/ipc/thread.h>
#include <arch/cortex.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>

#if (defined __ARM_ARCH_7M__) || (defined __ARM_ARCH_7EM__) \
	|| (defined __ARM_ARCH_8M_BASE__) || (defined __ARM_ARCH_8_1M_MAIN__) \
	|| (defined __ARM_ARCH_8_1M_BASE__) || (defined __ARM_ARCH_8M_MAIN__)

/** Lock futex.
 * Perform atomic futex lock. It is possible to lock futex which is either completely unlocked,
 * or a recursive futex, which has still some space left for locking. During locking, it is
 * checked if futex owner matches. If futex lock level is too deep or futex is owned by someone
 * else, then futex lock fails.
 * @param futex futex to be locked
 * @param thread_id identification of calling thread
 * @param max_depth maximum depth futex can already be locked in order to be still able to lock it
 * @returns 0 if futex lock was successful, 1 if locking failed for whatever reason
 */
int __futex_fast_lock(futex_t * futex, uint8_t thread_id, unsigned max_depth)
{
	unsigned state = __LDREXB(&futex->state);
	int success = FUTEX_FAILURE;
	if (state == 0 || (futex->owner == thread_id && state < max_depth))
	{
		state++;
		if ((success = __STREXB(state, &futex->state)) == 0)
		{
			// futex is claimed, mark us as the owner
			futex->owner = thread_id;
		}
	}
	__CLREX();
	return success;
}

/** Unlock futex.
 * This function performs fast unlock of futex if that is possible.
 * It first checks, if futex is locked by current thread and if
 * it is actually locked. If these conditions are met, then
 * futex unlock is performed.
 * @param futex Futex to be unlocked
 * @param thread_id Numeric identification of futex owner
 * @returns 0 if futex unlock was successful, 1 if unlocking failed for
 * whatever reason.
 */
int __futex_fast_unlock(futex_t * futex, uint8_t thread_id)
{
	uint8_t state = __LDREXB(&futex->state);
	int success = FUTEX_FAILURE;
	ASSERT(state > 0);
	if (state > 0)
	{
		if (futex->owner == thread_id)
		{
			state--;
			if ((success = __STREXB(state, &futex->state)) == 0)
			{
				if (state == 0)
				{
					// here the futex was entirely unlocked
					futex->owner = 0xFF;
				}
			}
		}
	}
	__CLREX();
	return success;
}

#endif

#ifdef __ARM_ARCH_6M__

#include <arch/sysenter.h>

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

#endif

int futex_init(futex_t * restrict futex)
{
	futex->owner = 0xFF;
	futex->state = 0;
	futex->flags = 0;
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
	futex->flags = 0;
	return 0;
}

/** @} */
