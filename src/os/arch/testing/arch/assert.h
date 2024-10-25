#pragma once

#include <stdlib.h>

#define ASSERT(cond) \
if (!(cond)) \
{\
    abort();\
}
