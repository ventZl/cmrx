#include "clock.h"

#include <cmrx/clock.h>

#include <signal.h>
#ifdef __APPLE__
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "linux.h"
#include <stdbool.h>
#include "thread.h"
#include <assert.h>

/* Linux port of CMRX has one default timing provider bundled with kernel */

/** Systick interval stored as value in ns */
static long int systick_ns = 0;

/** Systick interval stored as value in us */
static int systick_us = 0;

#ifndef __APPLE__
/** POSIX timer used to implement the systick */
static timer_t systick_timer;
#endif

void sigalrm_handler(int signo)
{
    assert(signo == SIGALRM);
    is_cmrx_thread();

    os_sched_timing_callback(systick_us);
    trigger_pendsv_if_needed();
}

void timing_provider_setup(int interval_ms)
{
    systick_ns = interval_ms * 100000;
    systick_us = interval_ms * 1000;

    /* SIGALRM has higher priority than SIGUSR1 (PendSV)
     */
    struct sigaction sigalrm_action = { 0 };
    sigalrm_action.sa_flags = 0;
    sigalrm_action.sa_handler = &sigalrm_handler;
    sigemptyset(&sigalrm_action.sa_mask);
    sigaddset(&sigalrm_action.sa_mask, SIGUSR1);

    sigaction(SIGALRM, &sigalrm_action, NULL);
}

void timing_provider_schedule(long delay_us)
{
    /* As of now this port is not tickless */
    (void) delay_us;

    static bool timer_started = false;

    if (!timer_started)
    {
        // expire in interval_ms and then every interval_ms
#ifdef __APPLE__
        struct itimerval systick = {{ systick_us, 0}, { 0, 0}};
        setitimer(ITIMER_REAL, &systick, NULL);
#else
        struct itimerspec systick = { { 0, systick_ns }, { 0, systick_ns } };
        struct sigevent timer_ev;
        timer_ev.sigev_signo = SIGALRM;
        timer_ev.sigev_notify = SIGEV_SIGNAL;
        timer_create(CLOCK_MONOTONIC, &timer_ev, &systick_timer);
        timer_settime(systick_timer, 0, &systick, NULL);
#endif
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
