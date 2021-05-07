#pragma once

#include <cmrx/defines.h>
#include <stdint.h>

void isr_kill(Thread_t thread_id, uint32_t signal);
