/** @defgroup util_assert Assertions
 *
 * @ingroup util
 *
 * CMRX SDK provides assertiong mechanism. This is fairly typical and very lean
 * assertion wrapper. Assertion evaluates condition. If it evaluates to false, then
 * debugger trap is signalized. This mechanism is available both in userspace and
 * kernel space and generally has no limitation on when and how can be used. Just 
 * mind the fact, that condition should generally be a constant expression, otherwise
 * code may behave differently, if assertions are turned off.
 *
 * Assertions are only built if NDEBUG macro is **not** defined.
 *
 * @{
 */
#pragma once

#ifndef NDEBUG

#ifndef TESTING
/** Evaluate condition and break if it evalues to false.
 */
#define ASSERT(cond) \
	if (!(cond)) \
		{\
			asm volatile("BKPT 0xFF\n\t");\
		}

#else

#include <stdlib.h>

#define ASSERT(cond) \
	if (!(cond)) \
	{\
		abort();\
	}

#endif

#else

#define ASSERT(cond)

#endif

/** @} */
