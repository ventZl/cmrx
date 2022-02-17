#pragma once

#include <libopencm3/cm3/cortex.h>

#define ALWAYS_INLINE __attribute__((always_inline))

#define cortex_disable_interrupts cm_disable_interrupts

ALWAYS_INLINE static inline void __DSB()
{
	asm volatile("DSB 0xF\n" : : : "memory");	
}

#warning "Find out when should which version be used"

ALWAYS_INLINE static inline void __ISB()
{
//	asm volatile("ISB\n" : : : "memory");	
	asm volatile("ISB 0xF\n" : : : "memory");	
}

#warning "Provide different versions for ARMv6M and ARMv7M"

ALWAYS_INLINE static inline void __ISR_return()
{
		asm volatile(
				"POP {R0, R1}\n\t"
				"MOV R12, R0\n\t"
				"MOV LR, R1\n\t"
// was:				"POP {R0, R1, R2, R3, R12, LR}\n\t"
				"POP {R0, R1, R2, R3}\n\t"
				"POP {R6, R7}\n\t"
				"BX R6\n\t"
				);
}

ALWAYS_INLINE static inline void __set_CONTROL(uint32_t control)
{
	asm volatile("MSR control, %0\n" : : "r" (control) : "memory");
}

/** Get value of process SP
 * @return top of application stack
 */
ALWAYS_INLINE static inline void * __get_PSP(void)
{
	void * psp;
	asm volatile(
			".syntax unified\n\t"
			"MRS %0, psp\n\t"
			: "=r" (psp) 
			);

	return psp;
}

/** Set value of process SP
 * @param stack_top new top of application stack
 */
ALWAYS_INLINE static inline void __set_PSP(unsigned long * stack_top)
{
	asm volatile(
			".syntax unified\n\t"
			"MSR psp, %0\n\t"
			: : "r" (stack_top)
	);
}

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


