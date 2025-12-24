#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <RTE_Components.h>
#include CMSIS_device_header

/** @defgroup arch_arm_cortex Cortex-M support routines
 * @ingroup arch_arm
 * Routines supporting access to Cortex-M core facilities.
 *
 * These routines allow kernel to perform some cricital tasks that the
 * architecture support layer needs to be able to execute in order to support
 * CMRX on Cortex-M.
 * @{ */

/** Exception frame *without* FPU context saved.
 */
typedef struct {
	uint32_t r0123[4];
/*	uint32_t r1;
	uint32_t r2;
	uint32_t r3;*/
	uint32_t r12;
	void (*lr)(void);
	void *pc;
	uint32_t xpsr;
/*	uint32_t arg4;
	uint32_t arg5;
	uint32_t arg6;
	uint32_t arg7;*/
} ExceptionFrame;

_Static_assert(sizeof(ExceptionFrame) % 8 == 0, "The size of ExceptionFrame structure is not divisible by 8!");

/** Exception frame *with* FPU context saved.
 */
typedef struct {
	uint32_t r0123[4];
	uint32_t r12;
	void (*lr)(void);
	void *pc;
	uint32_t xpsr;
	uint32_t fpu_regs[16];
	uint32_t fpscr;
	uint32_t __spacer;
} ExceptionFrameFP;

_Static_assert(sizeof(ExceptionFrameFP) % 8 == 0, "The size of ExceptionFrameFP structure is not divisible by 8!");

/** How many stack slots does exception frame without FPU occupy */
#define EXCEPTION_FRAME_ENTRIES			(sizeof(ExceptionFrame) / sizeof(uint32_t))
/** How many stack slots does exception frame with FPU occupy */
#define EXCEPTION_FRAME_FP_ENTRIES		(sizeof(ExceptionFrameFP) / sizeof(uint32_t))

#define ALWAYS_INLINE __STATIC_FORCEINLINE

/** Perform same actions as normal ISR return does.
 * When ISR performs return, then Cortex-M core does some specific steps to 
 * exit the ISR and return into thread whose execution has been interrupted.
 * The ISR frame stored in thread stack is loaded back into registers.
 * Perform the same steps.
 */
#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_8M_BASE__)

/* ARMv6M and ARMv8M-Baseline: limited register access */
ALWAYS_INLINE void __ISR_return()
{
		asm volatile(
				"POP {R0, R1, R2, R3}\n\t"
				"POP {R6, R7}\n\t"
				"MOV R12, R6\n\t"
				"MOV LR, R7\n\t"
				"POP {R6, R7}\n\t"
				"BX R6\n\t"
				);
}

#else

/* ARMv7M, ARMv7EM, ARMv8M-Mainline: full register access */
// TODO: Test this
ALWAYS_INLINE void __ISR_return()
{
		asm volatile(
				"POP {R0, R1, R2, R3}\n\t"
				"POP {R12, LR}\n\t"
				"POP {R6, R7}\n\t"
				"BX R6\n\t"
				);
}

#endif

/** Save application context.
 * This function will grab process SP and save 8 registers at the memory
 * location pointed by the PSP. Process SP is updated to point to the new
 * top of stack.
 * This operation will push 32 bytes (8 registers * 4 bytes) on stack.
 * @note This is defined as a macro so it can live inside naked functions.
 */
#define SAVE_CONTEXT() \
asm volatile( \
	".syntax unified\n\t" \
	"MRS r0, PSP\n\t" \
	"SUBS r0, #16\n\t" \
	"STMEA r0!, {r4 - r7}\n\t" \
	"SUBS r0, #32\n\t" \
	"MOV r4, r8\n\t" \
	"MOV r5, r9\n\t" \
	"MOV r6, r10\n\t" \
	"MOV r7, r11\n\t" \
	"STMEA r0!, {r4 - r7}\n\t" \
	"SUBS r0, #16\n\t" \
	"MSR PSP, r0\n\t" \
	: : : "r0", "r4", "r5", "r6", "r7", "memory" \
)


/** Load application context.
 * This function will grab process SP and load 8 registers from memory location
 * pointed by the PSP. Process SP is updated to point to the new top of the
 * stack.
 * This operation will pop 32 bytes (8 registers * 4 bytes) from stack.
 * @note This is defined as a macro so it can live inside naked functions.
 *
 * @param sp address where top of the stack containing application context is
 */
#define LOAD_CONTEXT() \
asm ( \
	".syntax unified\n\t" \
	"MRS r0, PSP\n\t" \
	"LDMFD r0!, {r4 - r7}\n\t" \
	"MOV r8, r4\n\t" \
	"MOV r9, r5\n\t" \
	"MOV r10, r6\n\t" \
	"MOV r11, r7\n\t" \
	"LDMFD r0!, {r4 - r7}\n\t" \
	"MSR PSP, r0\n\t" \
	: : : "r0", "r4", "r5", "r6", "r7", "memory" \
)

/** Retrieve address of n-th argument from exception frame.
 *
 * This function calculates address of n-th argument of function call from exception frame.
 * This is used whenever it is known, that exception frame was stored as an effect of
 * __SVC() call. It will automatically handle exception frame padding.
 * @param frame exception frame base address (usually value of SP)
 * @param argno number of argument retrieved
 * @returns address of argument relative to exception frame
 */

uint32_t * get_exception_arg_addr(ExceptionFrame * frame, unsigned argno, bool fp_active);

/** Retrieve value of exception frame function call argument.
 *
 * Retrieves value of n-th argument of function call calling __SVC()
 * @param frame exception frame base address
 * @param argno number of argument retrieved
 * @returns value of function argument
 */
static inline unsigned get_exception_argument(ExceptionFrame * frame, unsigned argno, bool fp_active)
{
	uint32_t * arg_addr = get_exception_arg_addr(frame, argno, fp_active);
	return *arg_addr;
}

/** Set value of exception frame function call argument.
 *
 * Sets value of n-th argument in exception frame.
 * @param frame exception frame base address
 * @param argno number of argument retrieved
 * @param value new value of function argument
 */
static inline void set_exception_argument(ExceptionFrame * frame, unsigned argno, unsigned value, bool fp_active)
{
	uint32_t * arg_addr = get_exception_arg_addr(frame, argno, fp_active);
	*arg_addr = value;
}

/** Configure PC and LR register values in exception frame.
 *
 * This function sets values for PC and LR upon usage of given exception frame.
 * @param frame exception frame base address
 * @param pc new value for PC register in exception frame
 * @param lr new value for LR register in exception frame
 */
static inline void set_exception_pc_lr(ExceptionFrame * frame, void * pc, void (* lr)(void))
{
	frame->pc = pc;
	frame->lr = lr;
}

/** Duplicate exception frame on thread's stack.
 * @param frame pointer of frame currently residing on top of process' stack
 * @param args amount of arguments pushed onto stack (first four come into R0-R3, fifth and following are pushed onto stack)
 * @param fp_active if true then floating point unit is actively used in the thread exception is for
 * @return address of duplicated exception frame
 */
ExceptionFrame * push_exception_frame(ExceptionFrame * frame, unsigned args, bool fp_active);

/** Creates space for additional arguments under exception frame.
 *
 * This function will move exception frame content args * 4 bytes lower. If resulting
 * address won't be 8-byt aligned, then additional alignment is applied to it.
 * Content of exception frame is copied automatically.
 * @param frame address of exception frame in memory
 * @param args amount of additional arguments for which space should be created under exception frame
 * @param fp_active if true then floating point unit is actively used in the thread exception is for
 * @returns address of shimmed exception frame.
 */
ExceptionFrame * shim_exception_frame(ExceptionFrame * frame, unsigned args, bool fp_active);

/** Remove exception frame from thread's stack.
 *
 * This function will revert effects of calling @ref push_exception_frame. It will handle
 * frame padding automatically.
 * @param frame exception frame base address
 * @param args number of function arguments passed onto stack (function args - 4)
 * @param fp_active if true then floating point unit is actively used in the thread exception is for
 * @return new address of stack top after frame has been removed from it
 */
ExceptionFrame * pop_exception_frame(ExceptionFrame * frame, unsigned args, bool fp_active);

/** Get value of process LR
 * @return top of application stack
 */
ALWAYS_INLINE void * __get_LR(void)
{
	void * lr;
	asm volatile(
			".syntax unified\n\t"
			"MOV %0, LR\n\t"
			: "=r" (lr)
			);

	return lr;
}

/** Set value of process LR
 * @param lr The new value of LR to set
 */
ALWAYS_INLINE void __set_LR(void * lr)
{
	asm volatile(
		".syntax unified\n\t"
		"MOV LR, %0\n\t"
		:
		: "r" (lr)
	);
}

/** Forges shutdown exception frame.
 * This exception frame sets CPU state to state similar to boot:
 * Privileged thread mode, MSP used as the stack.
 * Then exception return is used to load this frame and effectively
 * shutdown the remainder of the kernel.
 * After this function is executed, processor will continue running
 * code pointed to by @ref continue_here in privileged thread mode.
 */

#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_8M_BASE__)

/* ARMv6M and ARMv8M-Baseline: limited immediate support, use LDR */
ALWAYS_INLINE  void __forge_shutdown_exception_frame(void (*continue_here)(void))
{
	asm volatile(
		".syntax unified\n\t"
		"LDR R0, =0xFFFFFFF9\n\t" // return to thread mode with MSP
		"MOV LR, R0\n\t"
		"LDR R0, =0x01000000\n\t"
		"MOV R1, %0\n\t"
		"LDR R2, =0\n\t"
		"PUSH {R0}\n\t" // xPSR
		"PUSH {R1}\n\t" // PC
		"PUSH {R2}\n\t" // LR - NULL, point of no return
		"PUSH {R2}\n\t" // R12
		"PUSH {R2}\n\t" // R3
		"PUSH {R2}\n\t" // R2
		"PUSH {R2}\n\t" // R1
		"PUSH {R2}\n\t" // R0
		"BX LR"
		:
		: "r" (continue_here)
	);
}

#else

/* ARMv7M, ARMv7EM, ARMv8M-Mainline: modified immediate support */
ALWAYS_INLINE  __attribute__((noreturn)) void __forge_shutdown_exception_frame(void (*continue_here)(void))
{
	asm volatile(
		".syntax unified\n\t"
		"MOV R0, #0x01000000\n\t"
		"MOV R1, %0\n\t"
		"MOV R2, #0\n\t"
		"MOV LR, #0xFFFFFFF9\n\t" // return to thread mode with MSP
		"PUSH {R0}\n\t" // xPSR
		"PUSH {R1}\n\t" // PC
		"PUSH {R2}\n\t" // LR - NULL, point of no return
		"PUSH {R2}\n\t" // R12
		"PUSH {R2}\n\t" // R3
		"PUSH {R2}\n\t" // R2
		"PUSH {R2}\n\t" // R1
		"PUSH {R2}\n\t" // R0
		"BX LR"
		:
		: "r" (continue_here)
	);
}

#endif

static inline bool cortex_is_fpu_used(uint32_t return_addr)
{
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
	return (return_addr & EXC_RETURN_FTYPE) == EXC_RETURN_FTYPE;
#else
	return return_addr == EXC_RETURN_THREAD_PSP_FPU;
#endif
}

static inline bool cortex_is_thread_psp_used(uint32_t return_addr)
{
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
	return (return_addr & (EXC_RETURN_MODE | EXC_RETURN_SPSEL)) == (EXC_RETURN_MODE | EXC_RETURN_SPSEL);
#else
	return return_addr == EXC_RETURN_THREAD_PSP || return_addr == EXC_RETURN_THREAD_PSP_FPU;
#endif
}

/// @}
