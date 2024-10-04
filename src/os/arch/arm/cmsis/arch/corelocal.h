#pragma once
#include <RTE_Components.h>
#include CMSIS_device_header
#include <conf/kernel.h>
#include "cortex.h"

#define os_core_lock() __disable_irq()
#define os_core_unlock() __enable_irq()

#ifndef CMRX_ARCH_SMP_SUPPORTED

#   define coreid()	0
#   define OS_NUM_CORES	1
#   define os_smp_lock()
#   define os_smp_unlock()

#else

/** Callback to get the current CPU core number.
 * @returns ID of core executing this code. Starts with 0
 * @note It is implementor's task to provide body of this function as it is heavily HW-dependent.
 */
extern unsigned coreid();
extern void os_smp_lock();
extern void os_smp_unlock();

#ifndef OS_NUM_CORES
#error "Macro OS_NUM_CORES is not defined. Use -DOS_NUM_CORES=x to tell the CMRX kernel how many cores it manages!"
#endif

#endif
