#include "clock.h"

#include <cmrx/clock.h>

#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "linux.h"
#include <stdbool.h>


/* Linux port of CMRX has one default timing provider bundled with kernel */

/** Systick interval stored as value in ns */
static long int systick_ns = 0;

/** Systick interval stored as value in us */
static int systick_us = 0;

/** POSIX timer used to implement the systick */
static timer_t systick_timer;

void sigalrm_handler(int signo)
{
    os_sched_timing_callback(systick_us);
}

void timing_provider_setup(int interval_ms)
{
    systick_ns = interval_ms * 100000;
    systick_us = interval_ms * 1000;

    struct sigaction act = { 0 };
    act.sa_flags = 0;
    act.sa_handler = &sigalrm_handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);

    sigaction(SIGALRM, &act, NULL);
}

void timing_provider_schedule(long delay_us)
{
    /* As of now this port is not tickless */
    (void) delay_us;

    static bool timer_started = false;

    if (!timer_started)
    {
        // expire in interval_ms and then every interval_ms
        struct itimerspec systick = { { 0, systick_ns }, { 0, systick_ns } };
        struct sigevent timer_ev;
        timer_ev.sigev_signo = SIGALRM;
        timer_ev.sigev_notify = SIGEV_SIGNAL;
        timer_create(CLOCK_MONOTONIC, &timer_ev, &systick_timer);
        timer_settime(systick_timer, 0, &systick, NULL);
        timer_started = true;
    }
}

void timing_provider_delay(long delay_us)
{
    // We should make sure this won't call into CMRX usleep() syscall.
    usleep(delay_us);
}

long timing_get_current_cpu_freq(void)
{
    // What to return here?
    return 0;
}
