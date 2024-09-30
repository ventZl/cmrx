#include <extra/queue_server/queue_server.h>
#include <extra/queue_server/queue.h>

#include <cmrx/application.h>
#include <cmrx/rpc/implementation.h>
#include <cmrx/ipc/notify.h>

IMPLEMENTATION_OF(struct QueueObject, struct QueueInterface);

extern VTABLE struct QueueInterface queue_vtable;


static bool rpc_queue_send(INSTANCE(this), const unsigned char * data)
{
    bool rv = queue_send(&this->queue, data);
    if (rv) {
        notify_object(this);
    }
    return rv;
}

static bool rpc_queue_receive(INSTANCE(this), unsigned char * data)
{
    return queue_receive(&this->queue, data);
}

static bool rpc_queue_empty(INSTANCE(this), bool *empty)
{
    *empty = queue_empty(&this->queue);
    return true;
}

static void rpc_queue_destroy(INSTANCE(this))
{
    this->allocated = false;
    this->vtable = NULL;
}

VTABLE struct QueueInterface queue_vtable = {
    rpc_queue_send,
    rpc_queue_receive,
    rpc_queue_empty,
    rpc_queue_destroy
};

#include <cmrx/rpc/implementation.h>

IMPLEMENTATION_OF(struct QueueService, struct QueueServiceInterface);

static bool queue_server_create(INSTANCE(this), struct QueueObject ** queue, unsigned depth, unsigned size)
{
    for (unsigned q = 0; q < QUEUES_ALLOCATED; ++q)
    {
        if (this->queue[q].allocated == false)
        {
            queue_init(&this->queue[q].queue, depth, size);
            this->queue[q].vtable = &queue_vtable;
            this->queue[q].allocated = true;
            *queue = &this->queue[q];
            return true;
        }
    }
    return false;
}

static VTABLE const struct QueueServiceInterface queue_service_vtable = {
    queue_server_create
};

struct QueueService queue_service = {
    &queue_service_vtable,
    0
};

struct QueueService * queue_server = &queue_service;

OS_APPLICATION_MMIO_RANGES(queue_server, 0, 0, 0, 0);
OS_APPLICATION(queue_server);
