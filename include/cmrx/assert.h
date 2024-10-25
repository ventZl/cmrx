/** @defgroup libs Libraries
 * Optional libraries providing extended functionality.
 *
 * CMRX offers a small set of libraries that extend its functionality in some way.
 * Their use is purely optional, CMRX kernel does not need depend on them. Some of these
 * libraries provide new servers, while some others provide basic implementation of APIs 
 * which have to be implemented by the integrator normally.
 */

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

#include <arch/assert.h>

/** @} */
