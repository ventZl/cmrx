#include <stdbool.h>

bool timing_provider_delay_called = false;
long timing_provider_delay_us = -1;


void timing_provider_schedule(long delay_us)
{
    (void) delay_us;
}

void timing_provider_delay(long delay_us)
{
    timing_provider_delay_called = true;
    timing_provider_delay_us = delay_us;
}
