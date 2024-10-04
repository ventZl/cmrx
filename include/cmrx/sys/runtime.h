#pragma once

#include <arch/mpu.h>
#include <conf/kernel.h>


/** Prototype for thread entrypoint function.
 * Thread entrypoint function takes one user-defined argument. By default it is assumed
 * that this is a pointer to user data, but there is no checking performed on the value
 * passed to the callee. It can be anything, once properly typecasted.
 * Thread entrypoint can return signed 32bit value as it's return value. This is available
 * for whoever will call @ref thread_join() as thread exit status.
 */
typedef int (entrypoint_t)(void *);

/** MPU region description.
 */
struct OS_MPU_region {
    /** Start address. */
    void * start;
    /** End address. */
    void * end;
};

/** Static definition of process in firmware image.
 * This defines MPU regions of interest for this process,
 * such as data and BSS. Read-only data and code is all
 * readable.
 */
struct OS_process_definition_t {
    /** Static MPU region configuration for this process.
     */
    struct OS_MPU_region mpu_regions[OS_TASK_MPU_REGIONS];

    /** Ummmm */
    struct OS_MPU_region rpc_interface; /* this is not an actual MPU region */
};

/** Structure describing auto-spawned thread.
 *
 * This is a mechanism of creating threads without need to explicitly call
 * @ref thread_create(). Kernel will do that automatically upon main calling
 * @ref os_start().
 */
struct OS_thread_create_t {
    /** Owning process */
    const struct OS_process_definition_t * process;

    /** Entrypoint address */
    entrypoint_t * entrypoint;

    /** User data passed to entrypoint function */
    void * data;

    /** Thread priority */
    uint8_t priority;

    /** Core at which thread should be started */
    uint8_t core;
};
