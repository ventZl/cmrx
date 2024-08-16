#pragma once
/** @defgroup aux_cm_ipi Cortex-M generic IPI provider
 * @ingroup libs
 * This is the default provider of IPI for Cortex-M based processors.
 * This implementation provides the "slow" IPI, which means that there
 * are no interrupts occupied and implementation relies on all cores
 * running os_sched_yield() occasionally.
 * @{
 */

void os_ipi_sync_request();

void os_ipi_sync_release();

void os_ipi_sync_probe();

/** @} */

