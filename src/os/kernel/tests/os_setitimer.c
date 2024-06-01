#include <cmrx/os/timer.h>
#include <cmrx/os/runtime.h>
#include <ctest.h>
#include <arch/corelocal.h>

struct TimerEntry_t {
	uint32_t sleep_from;      ///< time at which sleep has been requested
	uint32_t interval;        ///< amount of time sleep shall take
	uint8_t thread_id;        ///< thread ID which requested the sleep
};


// Kernel private functions and variables, not part of any header
extern bool timing_provider_delay_called;
extern long timing_provider_delay_us;
extern struct OS_core_state_t core[OS_NUM_CORES];
extern struct TimerEntry_t sleepers[SLEEPERS_MAX];

CTEST_DATA(os_setitimer) {
};

CTEST_SETUP(os_setitimer) {
    timing_provider_delay_called = false;
    core[0].thread_current = 4;

    os_timer_init();
}

CTEST2(os_setitimer, add_interval_timer) {
    int rv = os_setitimer(10000);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);
    ASSERT_EQUAL(sleepers[0].interval, (1 << 31) | 10000U);
}

CTEST2(os_setitimer, cancel_interval_timer) {
    int rv = os_setitimer(10000);
    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(sleepers[0].thread_id, 4);

    rv = os_setitimer(0);

    ASSERT_EQUAL(rv, 0);
    ASSERT_EQUAL(timing_provider_delay_called, false);
    ASSERT_EQUAL(sleepers[0].thread_id, 0xFF);
}
