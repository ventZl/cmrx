#pragma once

#include <conf/kernel.h>

// These do nothing in testing environment
#define os_core_lock()
#define os_core_unlock()

#ifndef CMRX_ARCH_SMP_SUPPORTED

#define OS_NUM_CORES 1

#endif

typedef void (*callback_t)();

extern callback_t cmrx_smp_locked_callback;
extern callback_t cmrx_smp_unlocked_callback;


extern unsigned coreid();
extern void os_smp_lock();
extern void os_smp_unlock();

#ifndef OS_NUM_CORES
#error "Macro OS_NUM_CORES is not defined. Use -DOS_NUM_CORES=x to tell the CMRX kernel how many cores it manages!"
#endif

