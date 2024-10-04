/** @defgroup api_mutex Mutexes / Futexes
 * @ingroup api
 * Basic synchronization primitives for use inside one process.
 *
 * CMRX offers mutual exclusive access facilities for userspace processes.
 * There is an implementation of futex (fast userspace mutex) available and
 * supported. There is also an implementation of mutex (driven by kernel), but
 * this might get deprecated anytime soon.
 *
 * @ingroup api_mutexes 
 * @{
 */
#pragma once

#include <stdint.h>
#include <arch/sysenter.h>

#define MUTEX_INITIALIZED				1

/** Compile time initialization of futex.
 * If futex is initialized using this value, then it is not necessary
 * to call futex_init() during runtime.
 */
#define FUTEX_STATIC_INIT		{ 0xFF, 0, 0 }

/** Futex structure.
 * This is fast userspace mutex, which avoids calling kernel.
 * It provides basic functionality for locking, unlocking and
 * non-blocking lock. It can be single-issue, or recursive.
 */
typedef struct {
	uint8_t owner;
	uint8_t flags;
	uint8_t state;	
} futex_t;

/* Mutex structure.
 * It wraps futex, which is being operated from within
 * kernel context. Kernel is using futex internally, so
 * there is no need to use big kernel lock.
 */
typedef futex_t mutex_t;

/** Futexes
 * Futex is fast userspace mutex. Advantage of futex
 * over mutex is, that it can be locked and unlocked completely
 * from userspace. Futexes can be shared safely between threads
 * of single process, yet can't be normally shared between threads
 * of multiple processes unless explicitly placed into shared
 * memory region.
 */
int futex_init(futex_t * restrict futex);
int futex_destroy(futex_t * futex);
int futex_lock(futex_t * futex);
int futex_unlock(futex_t * futex);
int futex_trylock(futex_t * futex);

/** Mutexes
 * Mutexes are fully features inter-process locking primitive.
 * They are implemented as kernel system calls, so they are 
 * bit heavier to use than futexes. On the other hand, they can
 * be shared accross processes.
 */
__SYSCALL int mutex_init(mutex_t * restrict mutex);
__SYSCALL int mutex_destroy(mutex_t * mutex);
__SYSCALL int mutex_lock(mutex_t * mutex);
__SYSCALL int mutex_unlock(mutex_t * mutex);
__SYSCALL int mutex_trylock(mutex_t * mutex);

/** @} */
