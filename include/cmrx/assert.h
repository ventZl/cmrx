#pragma once

#ifndef NDEBUG

#ifndef TESTING
/** Evaluate condition and break if it evalues to false.
 */
#define ASSERT(cond) \
	if (!(cond)) \
		{\
			asm volatile("BKPT 0xFF\n\t");\
		}

#else

#include <stdlib.h>

#define ASSERT(cond) \
	if (!(cond)) \
	{\
		abort();\
	}

#endif

#else

#define ASSERT(cond)

#endif

