#include "riscv_rp2350_exception_frame.h"

uint32_t riscv_rp2350_exception_get_arg(const RiscvRp2350ExceptionFrame *frame, unsigned argno)
{
    switch (argno) {
    case 0:
        return frame->a0;
    case 1:
        return frame->a1;
    case 2:
        return frame->a2;
    case 3:
        return frame->a3;
    default:
        return 0;
    }
}

uint8_t riscv_rp2350_exception_get_syscall_id(const RiscvRp2350ExceptionFrame *frame)
{
    return (uint8_t)frame->a7;
}

void riscv_rp2350_exception_set_retval(RiscvRp2350ExceptionFrame *frame, int32_t retval)
{
    frame->a0 = (uint32_t)retval;
}

void riscv_rp2350_exception_advance_mepc_by_4(void)
{
    uint32_t mepc;

    __asm__ volatile(
        "csrr %0, mepc\n\t"
        : "=r"(mepc)
    );
    mepc += 4u;
    __asm__ volatile(
        "csrw mepc, %0\n\t"
        :
        : "r"(mepc)
    );
}
