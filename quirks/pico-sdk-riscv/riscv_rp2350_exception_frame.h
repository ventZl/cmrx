#pragma once

#include <stdint.h>

/*
 * RP2350 (Hazard3) machine exception frame as saved by Pico SDK
 * exception entry in external/pico-sdk/src/rp2_common/hardware_exception/exception_table_riscv.S.
 *
 * The layout below matches stores at offsets 0..60 bytes:
 *   0  : unused (ra is held in mscratch during handler execution)
 *   4  : t0
 *   8  : t1
 *   12 : t2
 *   16 : a0
 *   20 : a1
 *   24 : a2
 *   28 : a3
 *   32 : a4
 *   36 : a5
 *   40 : a6
 *   44 : a7
 *   48 : t3
 *   52 : t4
 *   56 : t5
 *   60 : t6
 */
typedef struct {
    uint32_t ra_slot_unused;
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
} RiscvRp2350ExceptionFrame;

_Static_assert(sizeof(RiscvRp2350ExceptionFrame) == (16u * sizeof(uint32_t)),
               "RP2350 exception frame must be 16 words");

uint32_t riscv_rp2350_exception_get_arg(const RiscvRp2350ExceptionFrame *frame, unsigned argno);
uint8_t riscv_rp2350_exception_get_syscall_id(const RiscvRp2350ExceptionFrame *frame);
void riscv_rp2350_exception_set_retval(RiscvRp2350ExceptionFrame *frame, int32_t retval);
void riscv_rp2350_exception_advance_mepc_by_4(void);
