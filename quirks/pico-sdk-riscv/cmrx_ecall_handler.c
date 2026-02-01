/*
 * CMRX RISC-V ecall (syscall) handler for Pico SDK (RP2350).
 *
 * Implements isr_riscv_machine_ecall_mmode_exception to handle syscalls
 * triggered by the ecall instruction from M-mode code.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <cmrx/arch/riscv/context_switch.h>
#include "riscv_rp2350_exception_frame.h"

/* Forward declaration - defined in kernel */
extern int os_system_call(uint32_t arg0, uint32_t arg1, uint32_t arg2,
                          uint32_t arg3, uint8_t syscall_id);

/*
 * RISC-V Privileged Specification:
 * - Table 3.6: mcause=11 identifies environment call from M-mode
 * - Section 3.1.14: mepc points at ecall; software advances by 4 to continue
 */

/*
 * C helper to dispatch the syscall.
 * Called from the assembly handler with a pointer to the exception frame.
 *
 * @param frame Pointer to the exception frame on stack
 * @return syscall return value (to be written to a0 in frame)
 */
int cmrx_ecall_dispatch(RiscvRp2350ExceptionFrame *frame)
{
    uint32_t arg0 = riscv_rp2350_exception_get_arg(frame, 0);
    uint32_t arg1 = riscv_rp2350_exception_get_arg(frame, 1);
    uint32_t arg2 = riscv_rp2350_exception_get_arg(frame, 2);
    uint32_t arg3 = riscv_rp2350_exception_get_arg(frame, 3);
    uint8_t syscall_id = riscv_rp2350_exception_get_syscall_id(frame);

    return os_system_call(arg0, arg1, arg2, arg3, syscall_id);
}

void cmrx_ecall_dispatch_writeback(RiscvRp2350ExceptionFrame *frame)
{
    int retval = cmrx_ecall_dispatch(frame);
    riscv_rp2350_exception_set_retval(frame, retval);
}

/*
 * RISC-V ecall exception handler.
 *
 * This overrides the weak isr_riscv_machine_ecall_mmode_exception
 * from Pico SDK's exception_table_riscv.S.
 *
 * When called by the exception dispatch code:
 * - sp points to the exception frame (caller-saved regs already saved)
 * - mepc points to the ecall instruction
 * - We must increment mepc by 4 before returning
 *
 * After dispatching the syscall, we call the context switch safe-point
 * to allow a pending context switch to occur (e.g., after usleep()).
 *
 * This handler is marked naked to have full control over the stack.
 *
 * See RISC-V Privileged Specification, Section 3.1.14 for mepc behavior.
 */
__attribute__((naked)) void isr_riscv_machine_ecall_mmode_exception(void)
{
    __asm__ volatile(
        /* Save ra - we'll make calls */
        "addi sp, sp, -16\n\t"
        "sw ra, 0(sp)\n\t"
        "sw s0, 4(sp)\n\t"

        /* Save exception frame pointer in s0 */
        /* Exception frame is at sp + 16 (our frame size) */
        "addi s0, sp, 16\n\t"

        /* Increment mepc by 4 to skip past ecall instruction */
        "call riscv_rp2350_exception_advance_mepc_by_4\n\t"

        /* Call C helper to dispatch syscall and write a0 into frame */
        /* a0 = exception frame pointer */
        "mv a0, s0\n\t"
        "call cmrx_ecall_dispatch_writeback\n\t"

        /* Call context switch safe-point */
        /* This allows pending context switches after syscalls like usleep() */
        "call os_riscv_context_switch_safe_point\n\t"

        /* Restore ra and s0, return to exception dispatcher */
        "lw ra, 0(sp)\n\t"
        "lw s0, 4(sp)\n\t"
        "addi sp, sp, 16\n\t"
        "ret\n\t"
        :
        :
        : "memory"
    );
}
