#include <cmrx/clock.h>

long current_cpu_freq = 0;

long timing_get_current_cpu_freq(void)
{
    return current_cpu_freq;
}
