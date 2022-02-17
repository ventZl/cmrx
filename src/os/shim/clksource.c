#include <cmrx/shim/clksource.h>

#if (defined STM32G4)
#include <libopencm3/stm32/rcc.h>

uint32_t sysclk_freq()
{
	return rcc_ahb_frequency;
}
#endif

#if (defined NRF51) || (defined NRF52)
uint32_t sysclk_freq()
{
	return 64000000U;
}
#endif

