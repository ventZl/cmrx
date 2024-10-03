#pragma once

/** @defgroup queue_server Queue RPC queue_server
 * @ingroup libs
 *
 * @{
 *
 * This is a userspace implementation of queues that can be used across processes.
 * CMRX does not offer queues as a standard syscall, unlike other RTOSes.
 *
 * If an application wants to use queues, it needs to include the queue server in
 * the build. This is done by calling:
 *
 * ~~~~~~
 * target_add_applications(<firmware> queue_server)
 * ~~~~~~
 *
 * into CMakeLists.txt.
 *
 * This will include the queue server and enable services described in this document.
 *
 * Queue server behaves as a pretty standard @ref rpc_intro "remote procedure call server".
 * You need a server instance in order to be able to work with queues. Queue server always
 * creates one publicly available instance.
 *
 * Application can call queue service to create a new queue such as:
 *
 * ~~~~~~~~~~
 * rpc_call(queue_server, create, ...)
 * ~~~~~~~~~~
 *
 * This will create new queue which can be used to call methods like send and receive.
 */

#include "queue.h"

#include <cmrx/rpc/interface.h>
#include <stdbool.h>
#include <stdint.h>

#define QUEUES_ALLOCATED 4

typedef uint8_t queue_id_t;

struct Queue;

/** Queue object interface.
 *
 * This API allows use of queue previously allocated via
 * @ref QueueServiceInterface.
 */
struct QueueInterface {
    /** Send data via the queue.
     *
     * This will copy data into queue. Queue must be initialized before
     * this call is made. The size of data is determined based on value
     * of `item_size` used during queue initialization.
     *
     * @param [in] this will be provided by the RPC call mechanism
     * @param [in] data pointer to data.
     * @returns true if data was copied into queue and false if queue is
     * already full
     */
    bool (*send)(INSTANCE(this), const unsigned char * data);

    /** Receive data from queue.
     *
     * This will copy the oldest data out of the queue. If queue is
     * empty then this call will block until queue is filled with at
     * least one entry of data.
     *
     * @param [in] this will be provided by the RPC call mechanism
     * @param [out] data pointer to place where data will be written
     * @returns true. Returns false in case spurious interrupt occurred
     * and queue is still empty.
     */
    bool (*receive)(INSTANCE(this), unsigned char * data);

    /** Returns queue status.
     *
     * @param [in] this will be provided by the RPC call mechanism
     * @returns true if queue is empty, false otherwise
     */
    bool (*empty)(INSTANCE(this), bool * out);

    /** Destroy queue.
     *
     * This will destroy this queue. Further calls to this queue
     * will fail. Queue can be allocated to another process in the
     * future.
     */
    void (*destroy)(INSTANCE(this));
};

struct QueueObject {
    const struct QueueInterface * vtable;
    bool allocated;
    struct Queue queue;
};

/** Queue Service interface.
 *
 * This is an interface of queue manager. It offers a method to allocate new queues.
 */
struct QueueServiceInterface {
    /** Create new queue.
     *
     * This method will allocate new queue instance, if one is available. Queue will be allocated
     * using properties determined by the call:
     * @param [out] queue pointer to place where handle to queue created will be saved
     * @param [in] depth amount of items queue should be able to store
     * @param [in] size size of one item
     * @returns true if queue was allocated, false if there is no free queue or queue does not
     * have sufficient storage for creating queue that can hold @ref depth items of @ref size bytes each.
     */
    bool (*create)(INSTANCE(this), struct QueueObject ** queue, unsigned depth, unsigned size);
};

/** Queue Server internals.
 *
 * This object is opaque to the userspace.
 */
struct QueueService {
    const struct QueueServiceInterface * vtable;
    struct QueueObject queue[QUEUES_ALLOCATED];

};

/** Queue server instance for RPC calls.
 *
 * This is publicly available instance of queue server. You can use it to call @ref rpc_call()
 * using interface defined in @ref QueueServiceInterface.
 */
extern struct QueueService * queue_server;

/** @} */
