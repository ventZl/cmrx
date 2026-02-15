#pragma once

#define ASSERT(cond) \
if (!(cond)) \
{\
    asm volatile("ebreak\n\t");\
}
