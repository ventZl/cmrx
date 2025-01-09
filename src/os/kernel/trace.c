#include "trace.h"
#include <cmrx/defines.h>

static struct TraceBuffer * event_trace_buffer = NULL;

#define TRACE_BUFFER_MAGIC  0xFACEFEED

bool trace_init(void * start_addr, void * end_addr)
{
    // Trace event_trace_bufferfer can only be initialized once
    if (event_trace_buffer != NULL)
    {
        return false;
    }

    event_trace_buffer = start_addr;
    event_trace_buffer->magic = TRACE_BUFFER_MAGIC;
    event_trace_buffer->most_recent_event = ~0;
    event_trace_buffer->last_possible_event = ((struct TraceEvent *) end_addr) - &event_trace_buffer->events[0];
    event_trace_buffer->wrapped = 0;

    return true;
}

void trace_event(enum TraceEventID event, uint32_t arg)
{
    if (event_trace_buffer == NULL)
    {
        return;
    }
    if (event_trace_buffer->magic != TRACE_BUFFER_MAGIC)
    {
        return;
    }
    event_trace_buffer->most_recent_event++;
    if (event_trace_buffer->most_recent_event > event_trace_buffer->last_possible_event)
    {
        event_trace_buffer->most_recent_event = 0;
        event_trace_buffer->wrapped++;
    }
    int id = event_trace_buffer->most_recent_event;
    event_trace_buffer->events[id].event_id = event;
    event_trace_buffer->events[id].event_arg = arg;
}

