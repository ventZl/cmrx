#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <conf/kernel.h>

/** @defgroup os_tracing Tracing Kernel Events
 * @ingroup os
 * Facilities to trace progress of kernel execution
 *
 * Sometimes the current state of system as inspected by the debugger
 * is not enough to fully understand what happens and why it happens.
 * For these rough days the kernel tracing infrastructure offers facility
 * to track certain events as they happened.
 *
 * If tracing infrastructure is enabled by CMake option then tracing
 * infrastructure support is built into kernel. This infrastructure is called
 * from certain places. As of now tracing is all or nothing. Once tracing
 * is enabled all trace events currently present in the code are recorded
 * if they happen.
 *
 * Trace buffer is a circular buffer that overwrites oldest recorded entries
 * if new events are traced and the buffer is not large enough to store them.
 *
 * @section os_tracing_enable Enabling tracing
 *
 * In order for tracing to be active the developer needs to turn on CMake
 * option named `CMRX_KERNEL_TRACING`. Then it is necessary to call function
 * @ref trace_init() which will tell the tracing infrastructure where the tracing
 * buffer can be stored.
 *
 * Tracing buffer is not allocated normally, rather it is developer's task to find
 * a suitable space outside of normally used memory where it can be placed.
 *
 * This buffer will be "formatted" during the initialization and subsequent calls
 * to @ref trace_event() will record events there.
 *
 * If buffer is not initialized then calls to @ref trace_event() do nothing.
 *
 * @section os_tracing_retrieving Retrieving trace data
 *
 * Trace data is stored in RAM. In order to process them one has to dump them to
 * the computer. This can be done in many ways. One possible way is to use the
 * provided GDB script. This script is available in CMRX kernel repository in
 * gdb/trace_log.gdb file. It provides dump_trace command. This command will dump
 * trace in human readable form into file `trace.log` into current working directory.
 *
 * @{
 */

enum TraceEventID {
    EVENT_NONE = 0,
    EVENT_CPU_BOOT,
    EVENT_THREAD_SWITCH,
    EVENT_SYSCALL,
    EVENT_NOTIFY_WAIT_INIT,
    EVENT_NOTIFY_WAITING,
    EVENT_NOTIFY_PENDING,
    EVENT_WAIT_PENDING,
    EVENT_WAIT_SUSPEND,
    EVENT_ISR_HANDLER,
    EVENT_KERNEL_TICK
};

/// @cond IGNORE
#ifndef CMRX_KERNEL_TRACING
#   define trace_init(start_addr, end_addr)
#   define trace_event(event, arg)
#else
/// @endcond
/** Initialize trace buffer.
 * Trace buffer is usually put outside of the allocated RAM so
 * it can occupy otherwise unused and unallocated RAM space.
 * @param start_addr start address of the trace buffer
 * @param end_addr last usable address by the trace buffer
 * @returns true if trace buffer has been initialized. False is returned
 * if trace buffer is already initialized.
 */
bool trace_init(void * start_addr, void * end_addr);

/** Trace event.
 * This will trace event into trace buffer.
 * @param event The event that happened
 * @param arg optional argument to the event, event-specific
 * @note If trace buffer was not initialized then this call does nothing.
 * Please mind that even then the call happens and consumes CPU cycles.
 */
void trace_event(enum TraceEventID event, uint32_t arg);
#endif

/** @} */
