#pragma once

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
    /** @copydoc queue_send */
    bool (*send)(INSTANCE(this), const unsigned char * data);

    /** @copydoc queue_receive */
    bool (*receive)(INSTANCE(this), unsigned char * data);

    /** @copydoc queue_empty */
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
    bool (*create)(INSTANCE(this), struct QueueObject ** queue, unsigned depth, unsigned size);
};

struct QueueService {
    const struct QueueServiceInterface * vtable;
    struct QueueObject queue[QUEUES_ALLOCATED];

};

extern struct QueueService * queue_server;
