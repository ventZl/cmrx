#pragma once

#define ASSERT(cond) \
if (!(cond)) \
{\
    asm volatile("BKPT 0xFF\n\t");\
}

