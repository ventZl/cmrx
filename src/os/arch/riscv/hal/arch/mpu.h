#pragma once

#include <conf/kernel.h>
#include <stdint.h>

/*
 * Stub MPU definitions required by the kernel headers.
 * Memory protection is not implemented by this HAL.
 */

struct MPU_Registers {
    uint32_t REG1;
    uint32_t REG2;
};

typedef struct MPU_Registers MPU_State[MPU_STATE_SIZE];

