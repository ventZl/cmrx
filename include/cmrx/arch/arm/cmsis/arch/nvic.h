#pragma once

#include <stdint.h>

int os_nvic_enable(uint32_t irq);
int os_nvic_disable(uint32_t irq);
