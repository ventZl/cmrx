#pragma once

/** Request Inter-processor synchronization.
 * This is minimalistic implementation for inter-processor synchronization 
 * request for Cortex-M processors. This implementation does not assume
 * presence of any inter-processor interrupt mechanism. It assumes that
 * each core checks for inter-processor sync every now and then.
 * This function will block until all online cores call os_ipi_sync_probe().
 * By doing so all cores remain synchronized and it is guarranteed they will
 * do nothing. Then this function returns.
 */
void os_ipi_sync_request();

/** Release cores waiting in inter-processor synchronization loop.
 * This function will release all cores which are waiting in os_ipi_sync_probe()
 * function waiting for core which requested synchronization to release it.
 * Failure to call this function after os_ipi_sync_request() was called will 
 * result in permanent halt of all other system cores.
 */
void os_ipi_sync_release();

/** Check if inter-processor synchronization is requested.
 * Each core should call this function from time to time. If any other core
 * calls for inter-processor synchronization, then core calling this function will
 * be blocked in the call until the core which requested the synchronization doesn't 
 * release it.
 */
void os_ipi_sync_probe();

