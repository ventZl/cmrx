#pragma once

#ifndef NDEBUG

/** Evaluate condition and break if it evalues to false.
 */
#define ASSERT(cond) \
	if (!(cond)) \
		{\
			asm volatile("BKPT 0xFF\n\t");\
		}

#else

#define ASSERT(cond)

#endif

