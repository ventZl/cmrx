#pragma once

#include "os.h"

#define __APPL_SYMBOL(application, symbol)	application ## _ ## symbol
#define ENTRYPOINT(application) void __APPL_SYMBOL(application, entry)(void)

#define OS_APPLICATION_MMIO_RANGE(application, from, to)\
	static void * const __APPL_SYMBOL(application, mmio_start) = (void *) (from);\
	static void * const __APPL_SYMBOL(application, mmio_end) = (void *) (to);

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
__attribute__((used)) __attribute__(( section(".applications") )) const struct OS_task_t __APPL_SYMBOL(application, instance) = {\
	__APPL_SYMBOL(application, entry),\
	{\
		{ &__APPL_SYMBOL(application, text_start), &__APPL_SYMBOL(application, text_end) },\
		{ &__APPL_SYMBOL(application, data_start), &__APPL_SYMBOL(application, data_end) },\
		{ &__APPL_SYMBOL(application, bss_start), &__APPL_SYMBOL(application, bss_end) },\
		{ __APPL_SYMBOL(application, mmio_start), __APPL_SYMBOL(application, mmio_end) }\
	},\
	NULL,\
	NULL\
	}
