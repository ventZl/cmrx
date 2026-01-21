#pragma once

#include <stdint.h>
#include "runtime.h"

#ifdef NDEBUG
#   define sanitize_psp(psp)
#else

/** Checks that PSP is within sane limits.
 *
 * This function asserts if PSP register contains value, which is
 * out of bounds for currently running thread.
 * @param PSP value of PSP register (actual, or intended)
 */
void sanitize_psp(uint32_t * psp);

#endif

#ifdef NDEBUG
#define sanitize_psp_for_thread(psp, thread_id)
#else

/** Checks that PSP is within limits for given thread.
 *
 * This function does the same as @ref sanitize_psp() yet with
 * explicitly set thread in whose context the PSP should be
 * checked.
 * @param PSP value of PSP register (actual, or intended)
 * @param thread_id ID of thread PSP should be checked for validity
 */
void sanitize_psp_for_thread(uint32_t * psp, Thread_t thread_id);

#endif
