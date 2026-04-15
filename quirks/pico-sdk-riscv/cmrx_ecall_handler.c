/*
 * CMRX RISC-V ecall (syscall) handler for Pico SDK (RP2350).
 *
 * Called by the CMRX exception dispatcher with a pointer to the full
 * ExceptionFrame.  Advances mepc past the ecall instruction
 * (RISC-V Privileged Specification, Section 3.1.14), dispatches the
 * syscall, and writes the return value into the frame's a0.
 *
 * The context switch safe-point is handled by the exception dispatcher,
 * not here.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <cmrx/arch/riscv/exception_frame.h>

extern int os_system_call(uint32_t arg0, uint32_t arg1, uint32_t arg2,
                          uint32_t arg3, uint8_t syscall_id);

void isr_riscv_machine_ecall_mmode_exception(ExceptionFrame *frame)
{
    frame->mepc += 4;

    uint32_t arg0 = riscv_exception_get_arg(frame, 0);
    uint32_t arg1 = riscv_exception_get_arg(frame, 1);
    uint32_t arg2 = riscv_exception_get_arg(frame, 2);
    uint32_t arg3 = riscv_exception_get_arg(frame, 3);
    uint8_t syscall_id = riscv_exception_get_syscall_id(frame);

    int retval = os_system_call(arg0, arg1, arg2, arg3, syscall_id);
    riscv_exception_set_retval(frame, retval);
}
