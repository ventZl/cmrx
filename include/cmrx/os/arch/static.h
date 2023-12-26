#pragma once

/** @defgroup arch Ports of CMRX to various platforms
 *
 * @ingroup os 
 *
 * Documentation of porting layers
 *
 */

/** @defgroup arch_arch Porting layer 
 * @ingroup arch
 * Description of CMRX porting layer. CMRX expects that the following functions are provided
 * by architecture support layer. If creating new layer, these functions have to be implemented.
 * @{
 */

/** Provides count of statically initialized threads.
 * @returns amount of threads that have to be statically initialized */
unsigned static_init_thread_count();

/** Provides address of statically initialized thread table.
 * @returns address of table containing details of statically initialized threads */
const struct OS_thread_create_t * static_init_thread_table();

/** Provides count of statically initialized processes
 * @returns amount of processes that have to be statically initialized */
unsigned static_init_process_count();

/** Provides address of statically intialized process table.
 * @returns address of table containins details of statically initialized processes */
const struct OS_process_definition_t * static_init_process_table();

/** @} */
