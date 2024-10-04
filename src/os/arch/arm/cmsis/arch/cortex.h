#pragma once
/** @defgroup arch_arm_cortex Cortex-M support routines
 * @ingroup arch_arm 
 * Routines supporting access to Cortex-M core facilities.
 *
 * These routines allow kernel to perform some cricital tasks that the
 * architecture support layer needs to be able to execute in order to support
 * CMRX on Cortex-M.
 * @{ */

#include <stdbool.h>
#include <stdint.h>
#include <RTE_Components.h>
#include CMSIS_device_header

/** Exception frame *without* FPU context saved.
 */
typedef struct {
	uint32_t r0123[4];
/*	uint32_t r1;
	uint32_t r2;
	uint32_t r3;*/
	uint32_t r12;
	void * lr;
	void * pc;
	uint32_t xpsr;
/*	uint32_t arg4;
	uint32_t arg5;
	uint32_t arg6;
	uint32_t arg7;*/
} ExceptionFrame;


#define ALWAYS_INLINE __STATIC_FORCEINLINE

/** Perform same actions as normal ISR return does.
 * When ISR performs return, then Cortex-M core does some specific steps to 
 * exit the ISR and return into thread whose execution has been interrupted.
 * The ISR frame stored in thread stack is loaded back into registers.
 * Perform the same steps.
 */
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
				"POP {R12, LR}\n\t"
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

/** Retrieve address of n-th argument from exception frame.
 *
 * This function calculates address of n-th argument of function call from exception frame.
 * This is used whenever it is known, that exception frame was stored as an effect of
 * __SVC() call. It will automatically handle exception frame padding.
 * @param frame exception frame base address (usually value of SP)
 * @param argno number of argument retrieved
 * @returns address of argument relative to exception frame
 */
static inline uint32_t * get_exception_arg_addr(ExceptionFrame * frame, unsigned argno)
{
	if (argno < 4)
	{
		return &(frame->r0123[argno]);
	}
	else
	{
		uint32_t * base = &frame->xpsr;
		if ((((*base) >> 9) & 1) == 1)
		{
			base += 2;
		}
		else
		{
			base += 1;
		}
		return &(base[argno - 4]);
	}
}

/** Retrieve value of exception frame function call argument.
 *
 * Retrieves value of n-th argument of function call calling __SVC()
 * @param frame exception frame base address
 * @param argno number of argument retrieved
 * @returns value of function argument
 */
static inline unsigned get_exception_argument(ExceptionFrame * frame, unsigned argno)
{
	uint32_t * arg_addr = get_exception_arg_addr(frame, argno);
	return *arg_addr;
}

/** Set value of exception frame function call argument.
 *
 * Sets value of n-th argument in exception frame.
 * @param frame exception frame base address
 * @param argno number of argument retrieved
 * @param value new value of function argument
 */
static inline void set_exception_argument(ExceptionFrame * frame, unsigned argno, unsigned value)
{
	uint32_t * arg_addr = get_exception_arg_addr(frame, argno);
	*arg_addr = value;
}

/** Configure PC and LR register values in exception frame.
 *
 * This function sets values for PC and LR upon usage of given exception frame.
 * @param frame exception frame base address
 * @param pc new value for PC register in exception frame
 * @param lr new value for LR register in exception frame
 */
static inline void set_exception_pc_lr(ExceptionFrame * frame, void * pc, void * lr)
{
	frame->pc = pc;
	frame->lr = lr;
}

#define EXCEPTION_FRAME_SIZE			8

/** Duplicate exception frame on thread's stack.
 * @param frame pointer of frame currently residing on top of process' stack
 * @param args amount of arguments pushed onto stack (first four come into R0-R3, fifth and following are pushed onto stack)
 * @return address of duplicated exception frame
 */
static inline ExceptionFrame * push_exception_frame(ExceptionFrame * frame, unsigned args)
{
	ExceptionFrame * outframe = (ExceptionFrame *) (((uint32_t *) frame) - (EXCEPTION_FRAME_SIZE + args));
	bool padding = false;

	// Check if forged frame is 8-byte aligned, or not
	if ((((uint32_t) outframe) % 8) != 0)
	{
		// Frame needs padding
		outframe = (ExceptionFrame *) (((uint32_t *) outframe) - 1);
		padding = true;
	}

	outframe->xpsr = frame->xpsr;

	if (padding)
	{
		// we have padded the stack frame, clear STKALIGN in order to let
		// CPU know that original SP was 4-byte aligned
		outframe->xpsr |= 1 << 9;
	}
	else
	{
		// we didn't pad the stack frame, set STKALIGN in order to let
		// CPU know that original SP was 8-byte aligned
		outframe->xpsr &= ~(1 << 9);
	}

	return outframe;
}

/** Creates space for additional arguments under exception frame.
 *
 * This function will move exception frame content args * 4 bytes lower. If resulting
 * address won't be 8-byt aligned, then additional alignment is applied to it.
 * Content of exception frame is copied automatically.
 * @param frame address of exception frame in memory
 * @param args amount of additional arguments for which space should be created under exception frame
 * @returns address of shimmed exception frame.
 */
static inline ExceptionFrame * shim_exception_frame(ExceptionFrame * frame, unsigned args)
{
	ExceptionFrame * outframe = (ExceptionFrame *) (((uint32_t *) frame) - args);
	bool padding = false;

	// Check if forged frame is 8-byte aligned, or not
	if ((((uint32_t) outframe) % 8) != 0)
	{
		// Frame needs padding
		outframe = (ExceptionFrame *) (((uint32_t *) outframe) - 1);
		padding = true;
	}

	/* This has to be done from lowest address to highest to avoid
	 * overwriting usable data.
	 */
	
	for (unsigned int q = 0; q < sizeof(ExceptionFrame) / 4; ++q)
	{
		((uint32_t*) outframe)[q] = ((uint32_t*) frame)[q];
	}
	if (padding)
	{
		// we have padded the stack frame, clear STKALIGN in order to let
		// CPU know that original SP was 4-byte aligned
		outframe->xpsr |= 1 << 9;
	}
	else
	{
		// we didn't pad the stack frame, set STKALIGN in order to let
		// CPU know that original SP was 8-byte aligned
		outframe->xpsr &= ~(1 << 9);
	}

	return outframe;
}

/** Remove exception frame from thread's stack.
 *
 * This function will revert effects of calling @ref push_exception_frame. It will handle
 * frame padding automatically.
 * @param frame exception frame base address
 * @param args number of function arguments passed onto stack (function args - 4)
 * @return new address of stack top after frame has been removed from it
 */
static inline ExceptionFrame * pop_exception_frame(ExceptionFrame * frame, unsigned args)
{
	ExceptionFrame * outframe = (ExceptionFrame *) (((uint32_t *) frame) + (EXCEPTION_FRAME_SIZE + args));
	if (((frame->xpsr >> 9) & 1) == 1)
	{
		outframe = (ExceptionFrame *) (((uint32_t *) outframe) + 1);
	}

	// rewrite xPSR from pop-ped frame, retain value of bit 9
	outframe->xpsr = (outframe->xpsr & (1 << 9)) | (frame->xpsr & ~(1 << 9));

	return outframe;
}

/** Get value of process LR
 * @return top of application stack
 */
ALWAYS_INLINE void * __get_LR(void)
{
	void * psp;
	asm volatile(
			".syntax unified\n\t"
			"MOV %0, LR\n\t"
			: "=r" (psp) 
			);

	return psp;
}



/// @}
