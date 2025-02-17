#include <extra/queue_server/queue.h>
#include <cmrx/ipc/notify.h>
#include <cmrx/defines.h>
#include <cmrx/assert.h>
#include <limits.h>

bool queue_send_silent(struct Queue * queue, const void * data)
{
    if (queue->read_cursor == queue->write_cursor && !queue->empty)
    {
        return false;
    }

    const unsigned size_limit = queue->depth * queue->item_size;

    uint8_t cursor = queue->write_cursor;
    for (uint8_t q = 0; q < queue->item_size; ++q) {
        queue->content[cursor] = ((unsigned char *)data)[q];
        cursor = (cursor + 1) % size_limit;
    }

    queue->write_cursor = cursor;
    queue->empty = false;

    return true;
}

bool queue_send(struct Queue * queue, const void * data)
{
    bool rv = queue_send_silent(queue, data);
    if (rv)
    {
        notify_object(queue);
    }
    return rv;
}

bool queue_receive_timeout(struct Queue * queue, void * data, uint32_t timeout_us)
{

    int rv = wait_for_object(queue, timeout_us);
    if (rv == E_TIMEOUT)
    {
        return false;
    }

    if (queue_empty(queue))
    {
        // Spurious interrupt?
        return false;
    }

    const unsigned size_limit = queue->depth * queue->item_size;

    uint8_t cursor = queue->read_cursor;
    for (uint8_t q = 0; q < queue->item_size; ++q) {
        ((unsigned char *)data)[q] = queue->content[cursor];
        cursor = (cursor + 1) % size_limit;
    }

    // Update `empty` flag before updating cursor position
    // otherwise we get a data hazard.
    if (cursor == queue->write_cursor)
    {
        queue->empty = true;
    }
    queue->read_cursor = cursor;

    return true;
}

bool queue_empty(struct Queue * queue)
{
    return (queue->write_cursor == queue->read_cursor) && queue->empty;
}

bool queue_full(struct Queue * queue)
{
    return (queue->write_cursor == queue->read_cursor) && !queue->empty;
}


bool queue_init(struct Queue * queue, uint8_t depth, uint8_t item_size)
{
    uint32_t alloc_size = depth * item_size;
    if (alloc_size > USHRT_MAX)
    {
        // Allocation size of the queue is larger than cursor range
        return false;
    }
    queue->empty = true;
    queue->write_cursor = 0;
    queue->read_cursor = 0;
    queue->item_size = item_size;
    queue->depth = depth;
    return true;
}
