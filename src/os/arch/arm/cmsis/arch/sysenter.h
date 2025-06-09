/** @ingroup os_syscall
 * @{
 */
#pragma once

#if (!defined TESTING)


/** Mark function as syscall entrypoint in userspace.
 * This gives the function some common attributes. Currently syscall entrypoint are
 * short functions which never get inlined and don't construct stack frame. This is
 * the most efficient method of calling syscalls right now.
 */
#define __SYSCALL		__attribute__((naked)) __attribute__((noinline))

#define ___SVC(no)\
	asm volatile(\
			"SVC %[immediate]\n\t"\
                  "BX LR\n\t" : : [immediate] "I" (no) : "r0")

/** Perform syscall.
 * @param no number of syscall.
 * @note ARM port will throw arguments to SVC call, they are already where they should be.
 */
#define __SVC(no, ...) ___SVC(no)

#else

#define __SYSCALL

void __SVC(uint8_t no);

#endif

/** @} */
