#pragma once

//#include "os.h"
#include <cmrx/os/runtime.h>
#include <stddef.h>

#define __APPL_SYMBOL(application, symbol)	application ## _ ## symbol

#define OS_APPLICATION_MMIO_RANGE(application, from, to)\
	static void * const __APPL_SYMBOL(application, mmio_start) = (void *) (from);\
	static void * const __APPL_SYMBOL(application, mmio_end) = (void *) (to)

#define OS_APPLICATION(application) \
extern void * __APPL_SYMBOL(application, text_start);\
extern void * __APPL_SYMBOL(application, text_end);\
extern void * __APPL_SYMBOL(application, data_start);\
extern void * __APPL_SYMBOL(application, data_end);\
extern void * __APPL_SYMBOL(application, bss_start);\
extern void * __APPL_SYMBOL(application, bss_end);\
extern void * __APPL_SYMBOL(application, __mmio_start);\
extern void * __APPL_SYMBOL(application, __mmio_end);\
\
__attribute__((externally_visible, used, section(".applications") )) const struct OS_process_t __APPL_SYMBOL(application, instance) = {\
	{\
		{ &__APPL_SYMBOL(application, text_start), &__APPL_SYMBOL(application, text_end) },\
		{ &__APPL_SYMBOL(application, data_start), &__APPL_SYMBOL(application, data_end) },\
		{ &__APPL_SYMBOL(application, bss_start), &__APPL_SYMBOL(application, bss_end) },\
		{ __APPL_SYMBOL(application, mmio_start), __APPL_SYMBOL(application, mmio_end) }\
	},\
	NULL,\
	NULL\
	}

#define OS_THREAD_CREATE(application, entrypoint, data, priority) \
__attribute__((externally_visible, used, section(".thread_create") )) const struct OS_thread_create_t __APPL_SYMBOL(application, thread_create_ ## entrypoint) = {\
	&__APPL_SYMBOL(application, instance),\
	entrypoint,\
	data,\
	priority\
}
