/** @defgroup api_init Static initialization
 *
 * @ingroup api
 *
 * Mechanisms provided for application designer to statically initialize objects.
 * These mechanisms are provided, so that various structures are generated into
 * application image. These replace need to call os routines manually. Currently
 * it is possible to statically initialize processes and threads.
 *
 */
/** @ingroup api_init
 * @{
 */
#pragma once

//#include "os.h"
#include <cmrx/os/runtime.h>
#include <stddef.h>

#define __APPL_SYMBOL(application, symbol)	application ## _ ## symbol
#define VTABLE __attribute__((section(".vtable."))) const
//#define __VTABLE1(app_name) __VTABLE2(application_name)
//#define VTABLE __VTABLE1(__COUNTER__)

#define OS_APPLICATION_MMIO_RANGE(application, from, to)\
	static void * const __APPL_SYMBOL(application, mmio_start) = (void *) (from);\
	static void * const __APPL_SYMBOL(application, mmio_end) = (void *) (to);\
	static void * const __APPL_SYMBOL(application, mmio_2_start) = (void *) 0;\
	static void * const __APPL_SYMBOL(application, mmio_2_end) = (void *) 0


#define OS_APPLICATION_MMIO_RANGES(application, from, to, from2, to2)\
	static void * const __APPL_SYMBOL(application, mmio_start) = (void *) (from);\
	static void * const __APPL_SYMBOL(application, mmio_end) = (void *) (to);\
	static void * const __APPL_SYMBOL(application, mmio_2_start) = (void *) (from2);\
	static void * const __APPL_SYMBOL(application, mmio_2_end) = (void *) (to2)


/** Declare userspace process.
 *
 * This will create userspace process entry in process table.
 * Process is used to contain information on MPU configuration all threads
 * bound to this process can use.
 */
#define OS_APPLICATION(application) \
extern void * __APPL_SYMBOL(application, data_start);\
extern void * __APPL_SYMBOL(application, data_end);\
extern void * __APPL_SYMBOL(application, bss_start);\
extern void * __APPL_SYMBOL(application, bss_end);\
extern void * __APPL_SYMBOL(application, vtable_start);\
extern void * __APPL_SYMBOL(application, vtable_end);\
extern void * __APPL_SYMBOL(application, __mmio_start);\
extern void * __APPL_SYMBOL(application, __mmio_end);\
extern void * __APPL_SYMBOL(application, shared_start);\
extern void * __APPL_SYMBOL(application, shared_end);\
\
__attribute__((externally_visible, used, section(".applications") )) const struct OS_process_definition_t __APPL_SYMBOL(application, instance) = {\
	{\
		{ &__APPL_SYMBOL(application, data_start), &__APPL_SYMBOL(application, data_end) },\
		{ &__APPL_SYMBOL(application, bss_start), &__APPL_SYMBOL(application, bss_end) },\
		{ __APPL_SYMBOL(application, mmio_start), __APPL_SYMBOL(application, mmio_end) },\
		{ __APPL_SYMBOL(application, mmio_2_start), __APPL_SYMBOL(application, mmio_2_end) },\
		{ &__APPL_SYMBOL(application, shared_start), &__APPL_SYMBOL(application, shared_end) }\
	},\
	{ &__APPL_SYMBOL(application, vtable_start), &__APPL_SYMBOL(application, vtable_end) }\
	}

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
#define OS_THREAD_CREATE(application, entrypoint, data, priority) \
__attribute__((externally_visible, used, section(".thread_create") )) const struct OS_thread_create_t __APPL_SYMBOL(application, thread_create_ ## entrypoint) = {\
	&__APPL_SYMBOL(application, instance),\
	entrypoint,\
	data,\
	priority\
}

/** @} */
