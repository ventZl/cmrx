#pragma once

#include <stdint.h>

int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

