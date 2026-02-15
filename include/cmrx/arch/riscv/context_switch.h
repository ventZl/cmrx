#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Context frame size keeps stack 16-byte aligned (riscv-abi documentation, 2.2 Hardware
 * Floating-point Calling Convention section).
 */
#define CMRX_RISCV_CONTEXT_FRAME_WORDS 16u
#define CMRX_RISCV_CONTEXT_FRAME_BYTES (CMRX_RISCV_CONTEXT_FRAME_WORDS * sizeof(uint32_t))

void os_riscv_context_switch_request(bool activate);
bool os_riscv_context_switch_is_pending(void);
void os_riscv_context_switch_safe_point(void);
