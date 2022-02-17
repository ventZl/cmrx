#pragma once

#warning "Make this more portable"

#ifdef RP2040

#include <libopencm3/rp2040/smp.h>

#define coreid()	SMP_CPUID
#define OS_NUM_CORES	2

#else

#define coreid()	0
#define OS_NUM_CORES	1

#endif
