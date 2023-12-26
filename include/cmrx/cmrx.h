#pragma once

/** @defgroup os Kernel documentation
 *
 * Following documents kernel internals. You should only ever be interested in this part of
 * CMRX if you intend to develop kernel server, extend CMRX kernel or you came accross a bug
 * or poorly documented part of kernel API.
 * @{
 */


/** Minimal header to be included into main.c to actually start up scheduler.
 */

extern void os_start();
