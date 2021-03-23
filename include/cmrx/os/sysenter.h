/** @ingroup os_syscall
 * @{
 */
#pragma once

#include <stdint.h>

#if (!defined TESTING)

/** Mark function as syscall entrypoint in userspace.
 * This gives the function some common attributes. Currently syscall entrypoint are
 * short functions which never get inlined and don't construct stack frame. This is
 * the most efficient method of calling syscalls right now.
 */
#define __SYSCALL		__attribute__((naked)) __attribute__((noinline))

#define ___SVC(no)\
	asm volatile(\
			"SVC " #no "\n\t"\
			"BX LR"\
			)

/** Perform syscall.
 * @param no number of syscall. This must be compile time deducible. You can't use
 * enumerations here. Only verbatim numbers and macros are permissible here.
 */
#define __SVC(no) ___SVC(no)

#else

#define __SYSCALL

void __SVC(uint8_t no);

#endif

/** @} */
