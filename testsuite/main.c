#include <cmrx/os/sched.h>
#include <debug.h>
#include <extra/systick.h>

int main(void)
{
    timing_provider_setup(1);
	os_start();
    TEST_FAIL();
    // This will never be called but it forces TEST_STEP to be 
    // linked into binary. This enables the generic GDB script to
    // proceed while adding breakpoint for test steps.
    TEST_STEP(0);
}

