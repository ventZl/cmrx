#pragma once

/*
 * RISC-V syscall implementation.
 *
 * Uses standard RISC-V syscall convention:
 * - Syscall number in a7
 * - Arguments in a0-a3 (already there from C calling convention)
 * - Return value in a0
 *
 * The ecall instruction causes mcause=11 (environment call from M-mode)
 * which is handled by isr_riscv_machine_ecall_mmode_exception.
 */

#define __SYSCALL __attribute__((naked)) __attribute__((noinline))

/*
 * Perform syscall via ecall instruction.
 * @param no syscall number (placed in a7 before ecall)
 *
 * The function arguments are already in a0-a3 per RISC-V calling convention.
 * We load the syscall number into a7, execute ecall, then return.
 * The ecall handler will place the return value in a0.
 */
#define __SVC(no, ...) \
    asm volatile( \
        "li a7, %[syscall_id]\n\t" \
        "ecall\n\t" \
        "ret\n\t" \
        : \
        : [syscall_id] "i" (no) \
        : "a7" \
    )

