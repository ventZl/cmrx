#pragma once

#include <stdint.h>

struct MPU_Registers {
    uint32_t REG1;
    uint32_t REG2;
};

typedef struct MPU_Registers MPU_State[MPU_STATE_SIZE];

