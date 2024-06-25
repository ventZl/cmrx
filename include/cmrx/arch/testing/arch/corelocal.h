#pragma once

#include <conf/kernel.h>

#ifndef CMRX_ARCH_SMP_SUPPORTED

#define coreid()	0
#define OS_NUM_CORES 1

#else

extern unsigned coreid();

#ifndef OS_NUM_CORES
#error "Macro OS_NUM_CORES is not defined. Use -DOS_NUM_CORES=x to tell the CMRX kernel how many cores it manages!"
#endif


#endif
