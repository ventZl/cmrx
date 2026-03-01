#pragma once

/** @defgroup arch_riscv_exception_frame RISC-V Exception Frame
 * @ingroup arch_riscv
 *
 * Unified exception frame for RISC-V trap handlers and context switching.
 *
 * This header defines the standard full-context frame saved on trap entry
 * and restored on trap exit.  It is the RISC-V equivalent of the ARM
 * ExceptionFrame in cortex.h.
 *
 * The layout covers all general-purpose registers that must be preserved
 * across a trap (x1/ra, x5-x31), plus the mepc and mstatus CSRs.
 * Registers x0 (zero), x2 (sp), x3 (gp), and x4 (tp) are excluded:
 * x0 is hardwired to zero; sp is saved separately in the thread control
 * block; gp and tp are program-global constants in M-mode.
 *
 * The frame is 32 words (128 bytes), keeping the stack 16-byte aligned
 * per the RISC-V psABI (riscv-abi documentation, section 2.1).
 *
 * References:
 *   - RISC-V Privileged Specification (mepc, mstatus, mcause)
 *   - RISC-V psABI (register conventions, stack alignment)
 * @{
 */

/*
 * Byte offsets — usable from both C and assembly.
 *
 * Caller-saved registers (offsets 0-60):
 */
#define EF_RA         0
#define EF_T0         4
#define EF_T1         8
#define EF_T2        12
#define EF_A0        16
#define EF_A1        20
#define EF_A2        24
#define EF_A3        28
#define EF_A4        32
#define EF_A5        36
#define EF_A6        40
#define EF_A7        44
#define EF_T3        48
#define EF_T4        52
#define EF_T5        56
#define EF_T6        60

/* Callee-saved registers (offsets 64-108): */
#define EF_S0        64
#define EF_S1        68
#define EF_S2        72
#define EF_S3        76
#define EF_S4        80
#define EF_S5        84
#define EF_S6        88
#define EF_S7        92
#define EF_S8        96
#define EF_S9       100
#define EF_S10      104
#define EF_S11      108

/* CSRs (offsets 112-116): */
#define EF_MEPC     112
#define EF_MSTATUS  116

/** Total frame size in bytes. */
#define EXCEPTION_FRAME_SIZE  128

/* Stringification helpers for use inside inline asm macros. */
#define _RISCV_EFS(x) #x
#define RISCV_EFS(x)  _RISCV_EFS(x)

/** Save application context.
 * Pushes a full ExceptionFrame onto the current stack: all GP registers
 * (ra, t0-t6, a0-a7, s0-s11) plus mepc and mstatus.
 * Uses t0/t1 as scratch for CSR reads (they are saved beforehand).
 * @note This is defined as a macro so it can live inside naked functions.
 */
#define SAVE_CONTEXT() \
asm volatile( \
	"addi sp, sp, -" RISCV_EFS(EXCEPTION_FRAME_SIZE) "\n\t" \
	"sw ra,  " RISCV_EFS(EF_RA) "(sp)\n\t" \
	"sw t0,  " RISCV_EFS(EF_T0) "(sp)\n\t" \
	"sw t1,  " RISCV_EFS(EF_T1) "(sp)\n\t" \
	"sw t2,  " RISCV_EFS(EF_T2) "(sp)\n\t" \
	"sw a0,  " RISCV_EFS(EF_A0) "(sp)\n\t" \
	"sw a1,  " RISCV_EFS(EF_A1) "(sp)\n\t" \
	"sw a2,  " RISCV_EFS(EF_A2) "(sp)\n\t" \
	"sw a3,  " RISCV_EFS(EF_A3) "(sp)\n\t" \
	"sw a4,  " RISCV_EFS(EF_A4) "(sp)\n\t" \
	"sw a5,  " RISCV_EFS(EF_A5) "(sp)\n\t" \
	"sw a6,  " RISCV_EFS(EF_A6) "(sp)\n\t" \
	"sw a7,  " RISCV_EFS(EF_A7) "(sp)\n\t" \
	"sw t3,  " RISCV_EFS(EF_T3) "(sp)\n\t" \
	"sw t4,  " RISCV_EFS(EF_T4) "(sp)\n\t" \
	"sw t5,  " RISCV_EFS(EF_T5) "(sp)\n\t" \
	"sw t6,  " RISCV_EFS(EF_T6) "(sp)\n\t" \
	"sw s0,  " RISCV_EFS(EF_S0) "(sp)\n\t" \
	"sw s1,  " RISCV_EFS(EF_S1) "(sp)\n\t" \
	"sw s2,  " RISCV_EFS(EF_S2) "(sp)\n\t" \
	"sw s3,  " RISCV_EFS(EF_S3) "(sp)\n\t" \
	"sw s4,  " RISCV_EFS(EF_S4) "(sp)\n\t" \
	"sw s5,  " RISCV_EFS(EF_S5) "(sp)\n\t" \
	"sw s6,  " RISCV_EFS(EF_S6) "(sp)\n\t" \
	"sw s7,  " RISCV_EFS(EF_S7) "(sp)\n\t" \
	"sw s8,  " RISCV_EFS(EF_S8) "(sp)\n\t" \
	"sw s9,  " RISCV_EFS(EF_S9) "(sp)\n\t" \
	"sw s10, " RISCV_EFS(EF_S10) "(sp)\n\t" \
	"sw s11, " RISCV_EFS(EF_S11) "(sp)\n\t" \
	"csrr t0, mepc\n\t" \
	"csrr t1, mstatus\n\t" \
	"sw t0,  " RISCV_EFS(EF_MEPC) "(sp)\n\t" \
	"sw t1,  " RISCV_EFS(EF_MSTATUS) "(sp)\n\t" \
	: : : "memory" \
)

/** Load application context.
 * Restores all GP registers and CSRs (mepc, mstatus) from the
 * ExceptionFrame on the current stack and pops the frame.
 * Does NOT execute mret — the caller handles that.
 * @note This is defined as a macro so it can live inside naked functions.
 */
#define LOAD_CONTEXT() \
asm volatile( \
	"lw t0,  " RISCV_EFS(EF_MEPC) "(sp)\n\t" \
	"lw t1,  " RISCV_EFS(EF_MSTATUS) "(sp)\n\t" \
	"csrw mepc, t0\n\t" \
	"csrw mstatus, t1\n\t" \
	"lw s0,  " RISCV_EFS(EF_S0) "(sp)\n\t" \
	"lw s1,  " RISCV_EFS(EF_S1) "(sp)\n\t" \
	"lw s2,  " RISCV_EFS(EF_S2) "(sp)\n\t" \
	"lw s3,  " RISCV_EFS(EF_S3) "(sp)\n\t" \
	"lw s4,  " RISCV_EFS(EF_S4) "(sp)\n\t" \
	"lw s5,  " RISCV_EFS(EF_S5) "(sp)\n\t" \
	"lw s6,  " RISCV_EFS(EF_S6) "(sp)\n\t" \
	"lw s7,  " RISCV_EFS(EF_S7) "(sp)\n\t" \
	"lw s8,  " RISCV_EFS(EF_S8) "(sp)\n\t" \
	"lw s9,  " RISCV_EFS(EF_S9) "(sp)\n\t" \
	"lw s10, " RISCV_EFS(EF_S10) "(sp)\n\t" \
	"lw s11, " RISCV_EFS(EF_S11) "(sp)\n\t" \
	"lw ra,  " RISCV_EFS(EF_RA) "(sp)\n\t" \
	"lw t0,  " RISCV_EFS(EF_T0) "(sp)\n\t" \
	"lw t1,  " RISCV_EFS(EF_T1) "(sp)\n\t" \
	"lw t2,  " RISCV_EFS(EF_T2) "(sp)\n\t" \
	"lw a0,  " RISCV_EFS(EF_A0) "(sp)\n\t" \
	"lw a1,  " RISCV_EFS(EF_A1) "(sp)\n\t" \
	"lw a2,  " RISCV_EFS(EF_A2) "(sp)\n\t" \
	"lw a3,  " RISCV_EFS(EF_A3) "(sp)\n\t" \
	"lw a4,  " RISCV_EFS(EF_A4) "(sp)\n\t" \
	"lw a5,  " RISCV_EFS(EF_A5) "(sp)\n\t" \
	"lw a6,  " RISCV_EFS(EF_A6) "(sp)\n\t" \
	"lw a7,  " RISCV_EFS(EF_A7) "(sp)\n\t" \
	"lw t3,  " RISCV_EFS(EF_T3) "(sp)\n\t" \
	"lw t4,  " RISCV_EFS(EF_T4) "(sp)\n\t" \
	"lw t5,  " RISCV_EFS(EF_T5) "(sp)\n\t" \
	"lw t6,  " RISCV_EFS(EF_T6) "(sp)\n\t" \
	"addi sp, sp, " RISCV_EFS(EXCEPTION_FRAME_SIZE) "\n\t" \
	: : : "memory" \
)

#ifndef __ASSEMBLER__

#include <stdint.h>

/** Full RISC-V trap context frame.
 *
 * Saved on the thread stack by every trap entry (timer ISR, exception
 * handler) and restored on trap exit.  When a context switch swaps SP
 * between save and restore, the restore operates on the new thread's
 * frame, launching it transparently.
 *
 * Initial thread stacks are populated with this same layout by
 * os_thread_populate_stack().
 */
typedef struct {
	/* Caller-saved registers */
	uint32_t ra;
	uint32_t t0;
	uint32_t t1;
	uint32_t t2;
	uint32_t a0;
	uint32_t a1;
	uint32_t a2;
	uint32_t a3;
	uint32_t a4;
	uint32_t a5;
	uint32_t a6;
	uint32_t a7;
	uint32_t t3;
	uint32_t t4;
	uint32_t t5;
	uint32_t t6;
	/* Callee-saved registers */
	uint32_t s0;
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t s4;
	uint32_t s5;
	uint32_t s6;
	uint32_t s7;
	uint32_t s8;
	uint32_t s9;
	uint32_t s10;
	uint32_t s11;
	/* CSRs */
	uint32_t mepc;
	uint32_t mstatus;
	/* Padding for 16-byte alignment */
	uint32_t _pad[2];
} ExceptionFrame;

_Static_assert(sizeof(ExceptionFrame) == EXCEPTION_FRAME_SIZE,
	       "ExceptionFrame size must match EXCEPTION_FRAME_SIZE");

/** How many uint32_t slots the exception frame occupies. */
#define EXCEPTION_FRAME_WORDS  (sizeof(ExceptionFrame) / sizeof(uint32_t))

/** Initial mstatus value for newly created threads.
 *
 * MPIE = 1 (bit 7): after mret, MIE is set so interrupts are enabled.
 * MPP  = 3 (bits 12:11): stay in M-mode after mret.
 *
 * These are standard RISC-V privileged specification bit positions.
 */
#define CMRX_RISCV_INITIAL_MSTATUS  ((1u << 7) | (3u << 11))

/** Retrieve a syscall argument from the exception frame.
 * @param frame exception frame pointer
 * @param argno argument index (0-3 maps to a0-a3)
 */
static inline uint32_t riscv_exception_get_arg(const ExceptionFrame *frame,
					       unsigned argno)
{
	switch (argno) {
	case 0: return frame->a0;
	case 1: return frame->a1;
	case 2: return frame->a2;
	case 3: return frame->a3;
	default: return 0;
	}
}

/** Retrieve the syscall ID from the exception frame (a7 per RISC-V ecall ABI). */
static inline uint8_t riscv_exception_get_syscall_id(const ExceptionFrame *frame)
{
	return (uint8_t)frame->a7;
}

/** Write the syscall return value into the exception frame (a0). */
static inline void riscv_exception_set_retval(ExceptionFrame *frame,
					      int32_t retval)
{
	frame->a0 = (uint32_t)retval;
}

#endif /* __ASSEMBLER__ */

/** @} */
