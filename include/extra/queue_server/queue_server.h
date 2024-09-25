#pragma once

#include <cmrx/rpc/interface.h>
#include <stdbool.h>
#include <stdint.h>

#define QUEUES_ALLOCATED 16
#define QUEUE_LENGTH    32

typedef uint8_t queue_id_t;

struct Queue;

struct QueueInterface {
    bool (*send)(INSTANCE(this), const uint32_t * data);
    bool (*receive)(INSTANCE(this), uint32_t * data);
    void (*destroy)(INSTANCE(this));
};

struct Queue {
    const struct QueueInterface * vtable;
    bool allocated;
    uint32_t content[QUEUE_LENGTH];
    volatile uint8_t write_cursor;
    volatile uint8_t read_cursor;
    volatile bool empty;
};

struct QueueServiceInterface {
    bool (*create)(INSTANCE(this), struct Queue ** queue);
};

struct QueueService {
    const struct QueueServiceInterface * vtable;
    struct Queue queue[QUEUES_ALLOCATED];

};

extern struct QueueService queue_server;
