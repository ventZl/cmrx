#include <cmrx/cmrx.h>
#include <debug.h>
#include <extra/systick.h>
#include <conf/kernel.h>

int main(void)
{
    timing_provider_setup(1);
#ifdef CMRX_ARCH_SMP_SUPPORTED
	os_start(0);
#else
    os_start();
#endif
    TEST_FAIL();
    // This will never be called but it forces TEST_STEP to be 
    // linked into binary. This enables the generic GDB script to
    // proceed while adding breakpoint for test steps.
    TEST_STEP(0);
}

