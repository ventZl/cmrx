#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <cmrx/arch/riscv/exception_frame.h>

void os_riscv_context_switch_request(bool activate);
bool os_riscv_context_switch_is_pending(void);
void os_riscv_context_switch_safe_point(void);
