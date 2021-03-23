#pragma once

#include <stdint.h>

__attribute__((always_inline)) static inline uint8_t __LDREXB(uint8_t * addr)
{
	uint8_t retval;
	asm volatile(
			"LDREXB %0, [%1]\n\t"
			: "=r" (retval)
			: "r" (addr)
			);
	return retval;
}

__attribute__((always_inline)) static inline int __STREXB(uint8_t * addr, uint8_t value)
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

__attribute__((always_inline)) static inline void __CLREX()
{
	asm volatile(
			"CLREX\n\t"
			);
}

/** Get value of process SP
 * @return top of application stack
 */
__attribute__((always_inline)) static inline void * __get_PSP(void)
{
	void * psp;
	asm volatile(
			".syntax unified\n\t"
			"MRS %0, psp\n\t"
			: "=r" (psp) 
			);

	return psp;
}

/** Get value of process LR
 * @return top of application stack
 */
__attribute__((always_inline)) static inline void * __get_LR(void)
{
	void * psp;
	asm volatile(
			".syntax unified\n\t"
			"MOV %0, LR\n\t"
			: "=r" (psp) 
			);

	return psp;
}

/** Set value of process SP
 * @param stack_top new top of application stack
 */
__attribute__((always_inline)) static inline void __set_PSP(unsigned long * stack_top)
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
__attribute__((always_inline)) static inline void * save_context()
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
__attribute__((always_inline)) static inline void load_context(uint32_t * sp)
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

__attribute__((always_inline)) static inline void __set_CONTROL(uint32_t control)
{
	asm volatile("MSR control, %0\n" : : "r" (control) : "memory");
}

__attribute__((always_inline)) static inline void __DSB()
{
	asm volatile("DSB 0xF\n" : : : "memory");	
}

__attribute__((always_inline)) static inline void __ISB()
{
	asm volatile("ISB 0xF\n" : : : "memory");	
}


