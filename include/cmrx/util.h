#pragma once

#include <cmrx/intrinsics.h>

#define LOG2(x)		(31 - __builtin_clz(x))
