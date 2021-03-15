#pragma once

#include <stdint.h>

bool schedule_context_switch(uint32_t current_task, uint32_t next_task);
