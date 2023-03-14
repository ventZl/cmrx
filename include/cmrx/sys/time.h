#pragma once

#include <stdint.h>

typedef uint32_t time_t;
typedef uint32_t suseconds_t;

/** Structure that represents time points with microsecond resolution.
 */
struct timeval {
    time_t      tv_sec;
    suseconds_t tv_usec;
};
