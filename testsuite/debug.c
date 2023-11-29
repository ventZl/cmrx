#include "debug.h"
#include <cmrx/assert.h>

void TEST_SUCCESS() {
	ASSERT(0);
}

void TEST_FAIL() {
	ASSERT(0);
}

void TEST_STEP(unsigned step)
{
    (void) step;
    return;
}

