#pragma once

#include <conf/kernel.h>

#ifndef CMRX_ARCH_SMP_SUPPORTED

#define coreid()	0
#define OS_NUM_CORES 1
#define os_smp_lock()
#define os_smp_unlock()

#else

typedef void (*callback_t)();
extern callback_t cmrx_smp_locked_callback;
extern callback_t cmrx_smp_unlocked_callback;


extern unsigned coreid();
extern void os_smp_lock();
extern void os_smp_unlock();

#ifndef OS_NUM_CORES
#error "Macro OS_NUM_CORES is not defined. Use -DOS_NUM_CORES=x to tell the CMRX kernel how many cores it manages!"
#endif


#endif
