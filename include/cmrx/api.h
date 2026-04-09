#pragma once

#include <conf/kernel.h>

#ifdef CMRX_VERBOSE_API_NAMES
#define CMRX_API(symname) cmrx_ ## symname
#else
#define CMRX_API(symname) symname
#endif

