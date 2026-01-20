#pragma once

#include <conf/kernel.h>

/* Portability layer header for <arch/...> includes. */

#ifndef CMRX_ARCH_SMP_SUPPORTED
#define OS_NUM_CORES 1
#endif

typedef void (*callback_t)();

extern unsigned coreid();
extern void os_core_lock();
extern void os_core_unlock();

extern void os_smp_lock();
extern void os_smp_unlock();

#ifndef OS_NUM_CORES
#error "Macro OS_NUM_CORES is not defined. Use -DOS_NUM_CORES=x to tell the CMRX kernel how many cores it manages!"
#endif

