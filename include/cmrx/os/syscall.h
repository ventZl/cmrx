#pragma once

#include <stdbool.h>

typedef int (* Syscall_Handler_t)(int, int, int, int);

struct Syscall_Entry_t {
	uint8_t id;
	Syscall_Handler_t handler;
};

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

static inline unsigned get_exception_argument(ExceptionFrame * frame, unsigned argno)
{
	uint32_t * arg_addr = get_exception_arg_addr(frame, argno);
	return *arg_addr;
}

static inline void set_exception_argument(ExceptionFrame * frame, unsigned argno, unsigned value)
{
	uint32_t * arg_addr = get_exception_arg_addr(frame, argno);
	*arg_addr = value;
}

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
