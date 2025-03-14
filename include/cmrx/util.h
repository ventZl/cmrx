/** @defgroup util Utilities
 *
 * This is set of small and generally "one can live without" utilities, which may make
 * one's life easier.
 */
#pragma once

#include <stddef.h>

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

/** Return pointer to enclosing container.
 *
 * This macro will provide a pointer to container enclosing
 * the member of certain type. Useful for traversing up the data
 * structure without having back-pointers.
 * @param ptr pointer to known container member
 * @param type type of the enclosing container
 * @param member name of the member inside enclosing container
 * @return pointer to the enclosing container
 */
#define CONTAINER_OF(ptr, type, member) ({\
        const typeof(((type *) 0)->member) * member_ptr = (ptr);\
        (type *) ((char *) member_ptr - offsetof(type, member));\
})



/** @} */
