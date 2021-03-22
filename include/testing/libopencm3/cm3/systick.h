#pragma once

#include <stdint.h>

#define STK_CSR_CLKSOURCE_AHB			1

static uint32_t STK_CVR;

void systick_set_clocksource(unsigned cs) {}
void systick_set_reload(unsigned rv) {}
void systick_counter_enable() {}
void systick_interrupt_enable() {}
