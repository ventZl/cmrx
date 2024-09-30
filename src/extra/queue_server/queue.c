#include <extra/queue_server/queue.h>

bool queue_send(struct Queue * queue, const unsigned char * data)
{
    if (queue->read_cursor == queue->write_cursor && !queue->empty)
    {
        return false;
    }

    uint8_t cursor = queue->write_cursor;
    for (uint8_t q = 0; q < queue->item_size; ++q) {
        queue->content[cursor] = data[q];
        cursor = (cursor + 1) % QUEUE_LENGTH;
    }

    queue->write_cursor = cursor;
    queue->empty = false;

    return true;
}

bool queue_receive(struct Queue * queue, unsigned char * data)
{
    if (queue->empty)
    {
        wait_for_object(queue, 0);
        if (queue->empty)
        {
            // Spurious interrupt?
            return false;
        }
    }

    uint8_t cursor = queue->read_cursor;
    for (uint8_t q = 0; q < queue->item_size; ++q) {
        data[q] = queue->content[cursor];
        cursor = (cursor + 1) % QUEUE_LENGTH;
    }

    queue->read_cursor = cursor;
    if (queue->read_cursor == queue->write_cursor)
    {
        queue->empty = true;
    }

    return true;
}

bool queue_empty(struct Queue * queue)
{
    return queue->empty;
}

bool queue_init(struct Queue * queue, uint8_t depth, uint8_t item_size)
{
    if (depth * item_size > QUEUE_LENGTH)
    {
        return false;
    }
    queue->empty = true;
    queue->write_cursor = 0;
    queue->read_cursor = 0;
    queue->item_size = item_size;
    queue->depth = depth;
    return true;
}
