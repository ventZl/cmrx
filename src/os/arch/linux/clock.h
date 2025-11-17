#pragma once

/** @defgroup arch_linux_timing Linux timing provider
 * @ingroup arch_linux
 *
 * This is the default (and the only) timing provider for the
 * Linux target platform. This timing provider is built around
 * SIGALRM being delivered based on timer that is set up on
 * timing provider start and first schedule.
 *
 * This in turn calls kernel callback for timing actions.
 *
 * Due to the way how this provider is integrated into kernel
 * it is made to be a fully integrated part of the kernel.
 * It is unlikely that anyone would ever need to replace it.
 *
 * @{
 */


/** Timer handler.
 * This handler will call the kernel timing callback.
 * @param signo for internal use
 */
void sigalrm_handler(int signo);

/**
 * @}
 */
