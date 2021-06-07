/** @defgroup util Utilities
 *
 * This is set of small and generally "one can live without" utilities, which may make
 * one's life easier.
 */
#pragma once

#include <cmrx/intrinsics.h>

/** @ingroup util
 * @{
 */

/** Returns integer lowest aproximation of log_2 for given value.
 *
 * Provides approximation of binary logarithm of given value. More specifically
 * returns log_2 of lowest set bit. This may be useful during bit unmasking.
 *
 * @param x value
 * @returns y such that 1 << y will set bit, which is lowest set bit in x
 */
#define LOG2(x)		(31 - __builtin_clz((uint32_t) x))

/** Returns length (count of items) in static array.
 *
 * This function is a shortcut for determining size of typed static array.
 * It is useful for determining amount of `for` iterations. Type of item
 * must be known at compile time.
 * @param obj statically allocated array of known type
 * @returns amount of items contained in an array
 */
#define LENGTH(obj)	(sizeof(obj)/sizeof(obj[0]))

/** @} */
