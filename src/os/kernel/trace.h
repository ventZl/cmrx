#pragma once

#include <cmrx/sys/trace.h>
#include <stdint.h>
#include <stdbool.h>

/** Kernel event tracing infrastructure */

struct __attribute__((packed)) TraceEvent {
    uint32_t event_id;
    uint32_t event_arg;
};

struct __attribute__((packed)) TraceBuffer {
    uint32_t magic;
    uint32_t last_possible_event;
    uint32_t most_recent_event;
    uint32_t wrapped;
    struct TraceEvent events[8192];
};
