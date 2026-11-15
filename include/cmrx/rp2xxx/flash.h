#pragma once

#include <stdint.h>
#include <arch/sysenter.h>
#include "rp2xxx.h"

int flash_erase(uint8_t * base, uint32_t length);

int flash_write(uint8_t * base, uint32_t length, const uint8_t * source);
