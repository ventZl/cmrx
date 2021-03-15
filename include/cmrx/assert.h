#pragma once

#ifndef NDEBUG

#define ASSERT(cond) \
	if (!(cond)) \
		{\
			asm volatile("BKPT 0xFF\n\t");\
		}

#else

#define ASSERT(cond)

#endif

