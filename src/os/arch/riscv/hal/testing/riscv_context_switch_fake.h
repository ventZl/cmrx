#pragma once

#include <stdbool.h>
#include <stdint.h>

extern bool riscv_context_switch_called;
extern bool riscv_context_switch_perform_switch;
extern uint32_t *riscv_context_switch_fake_sp;
