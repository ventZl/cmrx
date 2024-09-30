#pragma once

#include "queue.h"

#include <cmrx/rpc/interface.h>
#include <stdbool.h>
#include <stdint.h>

#define QUEUES_ALLOCATED 4

typedef uint8_t queue_id_t;

struct Queue;

struct QueueInterface {
    bool (*send)(INSTANCE(this), const unsigned char * data);
    bool (*receive)(INSTANCE(this), unsigned char * data);
    bool (*empty)(INSTANCE(this), bool * out);
    void (*destroy)(INSTANCE(this));
};

struct QueueObject {
    const struct QueueInterface * vtable;
    bool allocated;
    struct Queue queue;
};

struct QueueServiceInterface {
    bool (*create)(INSTANCE(this), struct QueueObject ** queue, unsigned depth, unsigned size);
};

struct QueueService {
    const struct QueueServiceInterface * vtable;
    struct QueueObject queue[QUEUES_ALLOCATED];

};

extern struct QueueService * queue_server;
