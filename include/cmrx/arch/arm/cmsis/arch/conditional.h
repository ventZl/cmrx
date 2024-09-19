/** @defgroup arch_arm_intrinsics ARM intrinsics
 *
 * @ingroup arch_arm 
 *
 * Various intrinsics. Mostly reimplemented CMSIS intrinsics for use in kernel code.
 * @{
 */
#pragma once

#include <RTE_Components.h>
#include CMSIS_device_header

#include <stdint.h>

#define ALWAYS_INLINE __attribute__((always_inline))

#if 0

/** Intrinsic access to ARM LDREXB instruction.
 * @param addr address to load from
 * @return 8-bit value read from addr
 */
ALWAYS_INLINE static inline uint8_t __LDREXB(uint8_t * addr)
{
	uint8_t retval;
	asm volatile(
			"LDREXB %0, [%1]\n\t"
			: "=r" (retval)
			: "r" (addr)
			);
	return retval;
}

/** Intrinsic access to ARM STREXB instruction.
 * @param addr address to store to
 * @param value 8-bit value to be stored to addr
 * @returns success flag
 */
ALWAYS_INLINE static inline int __STREXB(uint8_t * addr, uint8_t value)
{
	uint8_t status;
	asm volatile(
			"STREXB %0, %1, [%2]\n\t"
			: "=&r" (status)
			: "r" (value), "r" (addr)
			: "memory"
			);

	return status;
}

ALWAYS_INLINE static inline void __CLREX()
{
	asm volatile(
			"CLREX\n\t"
			);
}
#endif

#if 0
/** Save application context.
 * This function will grab process SP
 * This operation will claim 32 bytes (8 registers * 4 bytes) on stack.
 * @return top of application stack after application context was saved
 */
ALWAYS_INLINE static inline void * save_context()
{
	uint32_t * scratch;
	asm (
			".syntax unified\n\t"
			"MRS %0, PSP\n\t"
			"SUBS %0, #16\n\t"
			"STMEA %0!, {r4 - r7}\n\t"
			"SUBS %0, #32\n\t"
			"MOV r4, r8\n\t"
			"MOV r5, r9\n\t"
			"MOV r6, r10\n\t"
			"MOV r7, r11\n\t"
			"STMEA %0!, {r4 - r7}\n\t"
			"SUBS %0, #16\n\t"
			: "=r" (scratch)
			:
			: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
	);

	return scratch;
}

/** Load application context saved by save_context
 * from address sp.
 * @param sp address where top of the stack containing application context is
 */
ALWAYS_INLINE static inline void load_context(uint32_t * sp)
{
	asm (
			"LDMFD %0!, {r4 - r7}\n\t"
			"MOV r8, r4\n\t"
			"MOV r9, r5\n\t"
			"MOV r10, r6\n\t"
			"MOV r11, r7\n\t"
			"LDMFD %0!, {r4 - r7}\n\t"
			"MSR PSP, %0\n\t"
			:
			: [scratch] "r" (sp)
			: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
	);
}
#endif

/** @} */
