#pragma once
#include <RTE_Components.h>
#include CMSIS_device_header

#define ALWAYS_INLINE __STATIC_FORCEINLINE

#ifdef __ARM_ARCH_6M__

ALWAYS_INLINE void __ISR_return()
{
		asm volatile(
				"POP {R0, R1}\n\t"
				"MOV R12, R0\n\t"
				"MOV LR, R1\n\t"
				"POP {R0, R1, R2, R3}\n\t"
				"POP {R6, R7}\n\t"
				"BX R6\n\t"
				);
}

#else

// TODO: Test this
ALWAYS_INLINE void __ISR_return()
{
		asm volatile(
				"POP {R0, R1, R2, R3, R12, LR}\n\t"
				"POP {R0, R1, R2, R3}\n\t"
				"POP {R6, R7}\n\t"
				"BX R6\n\t"
				);
}

#endif

/** Save application context.
 * This function will grab process SP
 * This operation will claim 32 bytes (8 registers * 4 bytes) on stack.
 * @return top of application stack after application context was saved
 */
ALWAYS_INLINE void * save_context()
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
ALWAYS_INLINE void load_context(uint32_t * sp)
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



