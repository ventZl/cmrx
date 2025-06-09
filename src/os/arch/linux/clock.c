#include <cmrx/clock.h>

#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "linux.h"

/* Linux port of CMRX has one default timing provider bundled with kernel */

static long int systick_ns = 0;
static int systick_us = 0;
static timer_t systick_timer;

void sigalrm_handler(int signo, siginfo_t *info, void *context)
{
    os_sched_timing_callback(systick_us);
    trigger_pendsv_if_needed();
}

void timing_provider_setup(int interval_ms)
{
    systick_ns = interval_ms * 1000000;
    systick_us = interval_ms * 1000;

    struct sigaction act = { 0 };
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sigalrm_handler;

    sigaction(SIGALRM, &act, NULL);
    // expire in interval_ms and then every interval_ms
    struct itimerspec systick = { { 0, systick_ns }, { 0, systick_ns } };
    struct sigevent timer_ev;
    timer_ev.sigev_signo = SIGALRM;
    timer_ev.sigev_notify = SIGEV_SIGNAL;
    timer_create(CLOCK_MONOTONIC, &timer_ev, &systick_timer);
    timer_settime(systick_timer, 0, &systick, NULL);
}

void timing_provider_schedule(long delay_us)
{
    /* Intentionally left blank */
}

void timing_provider_delay(long delay_us)
{
    usleep(delay_us);
}

long timing_get_current_cpu_freq(void)
{
    return 0;
}
