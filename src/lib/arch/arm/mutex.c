/** @ingroup api_mutex
 * @{
 */

#include <RTE_Components.h>
#include CMSIS_device_header
#include <cmrx/ipc/mutex.h>
#include <cmrx/ipc/thread.h>
#include <cmrx/ipc/notify.h>
#include <cmrx/sys/notify.h>
#include <arch/cortex.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>

#if (defined __ARM_ARCH_7M__) || (defined __ARM_ARCH_7EM__) \
	|| (defined __ARM_ARCH_8M_BASE__) || (defined __ARM_ARCH_8_1M_MAIN__) \
	|| (defined __ARM_ARCH_8_1M_BASE__) || (defined __ARM_ARCH_8M_MAIN__)

/** Lock futex.
 * Perform atomic futex lock. It is only possible to lock an unlocked mutex. Locking may fail
 * spuriously due to the hardware specifics (such as load/store conditional). Recursive mutexes
 * are not supported as of now.
 * @param futex futex to be locked
 * @param thread_id identification of calling thread
 * @returns 0 if futex lock was successful, 1 if locking failed for whatever reason
 */
int __futex_fast_lock(futex_t * futex, uint8_t thread_id)
{
	unsigned state = __LDREXB(&futex->state);
	int success = FUTEX_FAILURE;
	if (state == 0)
	{
		state++;
		if ((success = __STREXB(state, &futex->state)) == FUTEX_SUCCESS)
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
 * If mutex is not locked, this action will fail. It is an undefined
 * behavior to unlock mutex locked by another thread.
 * @param futex Futex to be unlocked
 * @param thread_id Numeric identification of futex owner
 * @returns 0 if futex unlock was successful, 1 if unlocking failed for
 * whatever reason.
 */
int __futex_fast_unlock(futex_t * futex)
{
	unsigned state = __LDREXB(&futex->state);
	int success = FUTEX_FAILURE;
	if (state > 0)
	{
		ASSERT(futex->owner == thread_id);

		state--;
		success = __STREXB(state, &futex->state);
	}
	__CLREX();
	return success;
}

#endif

#ifdef __ARM_ARCH_6M__

int __futex_fast_lock(futex_t * futex, uint8_t thread_id)
{
	int rv = wait_for_object_value(&futex->state, 0, 0, NOTIFY_VALUE_INCREMENT);
	if (rv == E_OK_NO_WAIT)
	{
		futex->owner = thread_id;
		return FUTEX_SUCCESS;
	}
	return FUTEX_FAILURE;
}

int __futex_fast_unlock(futex_t * futex)
{
	unsigned state = futex->state;
	int success = FUTEX_FAILURE;
	if (state > 0)
	{
		ASSERT(futex->owner == thread_id);
		state--;
		futex->state = state;
		success = FUTEX_SUCCESS;
	}
	return success;
}

#endif

int futex_init(futex_t * restrict futex)
{
	ASSERT(futex != NULL);
	futex->owner = 0xFF;
	futex->state = 0;
	return 0;
}

int futex_lock(futex_t * futex)
{
	ASSERT(futex != NULL);
	uint8_t thread_id = get_tid();
	int success;
	do {
		success = __futex_fast_lock(futex, thread_id);
		if (success != FUTEX_SUCCESS)
		{
			wait_for_object_value(&futex->state, 0, 0, NOTIFY_PRIORITY_INHERIT(futex->owner));
		}
	} while (success != FUTEX_SUCCESS);
	return 0;
}

int futex_trylock(futex_t * futex)
{
	ASSERT(futex != NULL);
	uint8_t thread_id = get_tid();
	int success = __futex_fast_lock(futex, thread_id);
	return success;
}

int futex_unlock(futex_t * futex)
{
	ASSERT(futex != NULL);
	int success;
	do {
		success = __futex_fast_unlock(futex);
		if (success == FUTEX_SUCCESS)
		{
			notify_object2(&futex->state, NOTIFY_PRIORITY_DROP);
		}
	} while (success != FUTEX_SUCCESS);
	return 0;
}

int futex_destroy(futex_t* futex)
{
	ASSERT(futex != NULL);
	futex->state = 0;
	futex->owner = 0xFF;
	return 0;
}

/** @} */
