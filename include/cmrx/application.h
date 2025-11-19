#pragma once

#include <cmrx/sys/runtime.h>
#include <conf/kernel.h>
#include <arch/application.h>
#include <stddef.h>

#define __APPL_SYMBOL(application, symbol)	application ## _ ## symbol

#define _OS_THREAD_CREATE(application, entrypoint, data, priority, core) \
CMRX_THREAD_AUTOCREATE_CONSTRUCTOR(application, entrypoint, data, priority, core)

/** @defgroup api_init Static initialization
 * @ingroup api
 * Mechanisms provided for application designer to statically initialize objects.
 * These mechanisms are provided, so that various structures are generated into
 * application image. These replace need to call os routines manually. Currently
 * it is possible to statically initialize processes and threads.
 *
 */
/** @ingroup api_init
 * @{
 */

/** Define structure to be a VTABLE definition.
 * CMRX only allows remote procedure calls to objects whose vtable
 * pointers point to structures that are considered VTable definitions.
 *
 * Mark every VTABLE structure instance with this macro to make it work
 * for remote procedure calls.
 */
#define VTABLE CMRX_VTABLE_SPECIFIER

/** Define the MMIO range accessible by the said application.
 * This macro allows to define a range of addresses accessible by the application
 * code. Typical use is when user-space drivers are given access to certain
 * address range where a peripheral resides.
 */
#define OS_APPLICATION_MMIO_RANGE(application, from, to)\
	static void * const __APPL_SYMBOL(application, mmio_start) = (void *) (from);\
	static void * const __APPL_SYMBOL(application, mmio_end) = (void *) (to);\
	static void * const __APPL_SYMBOL(application, mmio_2_start) = (void *) 0;\
	static void * const __APPL_SYMBOL(application, mmio_2_end) = (void *) 0;\
	_Static_assert(from == 0 || to == 0 || from % (to - from) == 0, "MMIO range not size-aligned")

/** Define two MMIO range accessible by the said application.
 * This macro allows to define a range of addresses accessible by the application
 * code. Typical use is when user-space drivers are given access to certain
 * address range where a peripheral resides.
 *
 * This macro differs from @ref OS_APPLICATION_MMIO_RANGE only by the fact
 * that it allows to pass two ranges instead of one.
 */

#define OS_APPLICATION_MMIO_RANGES(application, from, to, from2, to2)\
	static void * const __APPL_SYMBOL(application, mmio_start) = (void *) (from);\
	static void * const __APPL_SYMBOL(application, mmio_end) = (void *) (to);\
	static void * const __APPL_SYMBOL(application, mmio_2_start) = (void *) (from2);\
	static void * const __APPL_SYMBOL(application, mmio_2_end) = (void *) (to2);\
	_Static_assert(from == 0 || to == 0 || from % (to - from) == 0, "MMIO range 1 not size-aligned");\
    _Static_assert(from == 0 || to == 0 || from2 == 0 || to2 == 0 || from2 % (to2 - from2) == 0, "MMIO range 2 not size-aligned")


/** Declare userspace process.
 *
 * This will create userspace process entry in process table.
 * Process is used to contain information on MPU configuration all threads
 * bound to this process can use.
 */
#define OS_APPLICATION(application) \
CMRX_APPLICATION_INSTANCE_CONSTRUCTOR(application)

/** Thread autostart facility.
 *
 * Use this to automatically create thread upon kernel start.
 * With this you don't need to call @ref thread_create() explicitly. Kernel will create
 * and initialize thread for you.
 * @param application name of process/application you want to bind your thread to.
 * @param entrypoint entrypoint function name. This function will be given control once thread starts.
 * @param data user-defined data passed to entrypoint function.
 * @param priority thread priority of newly created thread
 */
#ifdef CMRX_ARCH_SMP_SUPPORTED
#define OS_THREAD_CREATE(application, entrypoint, data, priority, core) \
    _OS_THREAD_CREATE(application, entrypoint, data, priority, core)
#else
#define OS_THREAD_CREATE(application, entrypoint, data, priority) \
    _OS_THREAD_CREATE(application, entrypoint, data, priority, 0)
#endif


/** @} */
