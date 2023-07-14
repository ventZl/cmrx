#pragma once

/** @addtogroup api_rpc */

/** Syntactic sugar to make pointer to instance to stand out
 * Internally, the pointer to instance is just a void pointer from
 * the caller's perspective. This has to be shadowed during the 
 * implementation.
 * In order for both interface and implementation to look the same
 * there is this macro, which only declared its argument as a void
 * pointer.
 */

#ifndef INSTANCE
#define INSTANCE(var) void * var
#endif
