#pragma once

#include <stdint.h>

/** Checks that PSP is within sane limits.
 *
 * This function asserts if PSP register contains value, which is
 * out of bounds for currently running thread.
 * @param PSP value of PSP register (actual, or intended)
 */
void sanitize_psp(uint32_t * psp);

