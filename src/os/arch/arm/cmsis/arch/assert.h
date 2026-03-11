#pragma once

#ifndef NDEBUG
#   define ASSERT(cond) \
if (!(cond)) \
{\
    asm volatile("BKPT 0xFF\n\t");\
    __builtin_unreachable();\
}
#else
#   define ASSERT(cond)
#endif

