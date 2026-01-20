#pragma once

/* Syscall/trap entry is platform-specific. */
#define __SYSCALL
#define __SVC(x) return 0;

