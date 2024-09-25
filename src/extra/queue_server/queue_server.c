#include <extra/queue_server/queue_server.h>

#include <cmrx/application.h>
#include <cmrx/rpc/implementation.h>
#include <cmrx/ipc/notify.h>

IMPLEMENTATION_OF(struct Queue, struct QueueInterface);

extern VTABLE struct QueueInterface queue_vtable;

static bool queue_send(INSTANCE(this), const uint32_t * data)
{
    if (this->read_cursor == this->write_cursor && !this->empty)
    {
        return false;
    }

    this->content[this->write_cursor] = *data;
    this->write_cursor = (this->write_cursor + 1) % QUEUE_LENGTH;
    this->empty = false;

    notify_object(this);

    return true;
}

static bool queue_receive(INSTANCE(this), uint32_t * data)
{
    if (this->empty)
    {
        wait_for_object(this, 0);
        if (this->empty)
        {
            // Spurious interrupt?
            return false;
        }
    }

    *data = this->content[this->read_cursor];
    this->read_cursor = (this->read_cursor + 1) % QUEUE_LENGTH;
    if (this->read_cursor == this->write_cursor)
    {
        this->empty = true;
    }

    return true;
}

static void queue_destroy(INSTANCE(this))
{
    this->allocated = false;
    this->vtable = NULL;
}

VTABLE const struct QueueInterface queue_vtable = {
    queue_send,
    queue_receive,
    queue_destroy
};

#include <cmrx/rpc/implementation.h>

IMPLEMENTATION_OF(struct QueueService, struct QueueServiceInterface);

static bool queue_server_create(INSTANCE(this), struct Queue ** queue)
{
    for (unsigned q = 0; q < QUEUES_ALLOCATED; ++q)
    {
        if (this->queue[q].allocated == false)
        {
            this->queue[q].allocated = true;
            this->queue[q].vtable = &queue_vtable;
            this->queue[q].empty = true;
            this->queue[q].write_cursor = 0;
            this->queue[q].read_cursor = 0;
            *queue = &this->queue[q];
            return true;
        }
    }
    return false;
}

static VTABLE const struct QueueServiceInterface queue_service_vtable = {
    queue_server_create
};

VTABLE struct QueueService queue_service = {
    &queue_service_vtable,
    0
};

OS_APPLICATION_MMIO_RANGES(queue_server, 0, 0, 0, 0);
OS_APPLICATION(queue_server);
