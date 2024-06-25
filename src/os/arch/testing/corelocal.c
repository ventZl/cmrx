#include <arch/corelocal.h>

unsigned cmrx_current_core = 0;

unsigned coreid() { return cmrx_current_core; }
