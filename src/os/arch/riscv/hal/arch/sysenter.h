#pragma once

#include <cmrx/defines.h>

/* Syscall/trap entry is platform-specific. */
#define __SYSCALL
#define __SVC(x) return E_NOTAVAIL;

