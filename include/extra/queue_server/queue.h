#pragma once

/** @defgroup aux_queue Queue library
 * @ingroup libs
 * This is auxiliary implementation of queue. CMRX does not offer queues
 * as an inherent service provided by the kernel as this does not fit the
 * microkernel nature of CMRX RTOS.
 *
 * CMRX natively supports lower communication and synchronization primitives
 * than queues.
 *
 * This implementation offers single producer-single consumer safe
 * implementation of queue that is usable even from interrupt context.
 *
 * This library will find uses if you need to implement queue between threads
 * of single process and/or you need to push data into queue from ISR handler.
 *
 * If you need a queue that works across processes then see @ref aux_queue_server
 * which wraps this library into RPC interface and thus is usable globally.
 * @{
 */

#include <stdbool.h>
#include <stdint.h>

#define QUEUE_LENGTH    256

/** Queue structure */
struct Queue {
    unsigned char content[QUEUE_LENGTH];
    volatile uint8_t write_cursor;
    volatile uint8_t read_cursor;
    volatile bool empty;
    uint8_t depth;
    uint8_t item_size;

};

/** Initialize queue structure.
 *
 * This call will initialize queue structure. It does not examine previous
 * state of the structure, thus any previous existing state is erased.
 *
 * @param [in] queue pointer to queue being initialized
 * @param [in] depth amount of entries queue should be able to store
 * @param [in] item_size size of each item in bytes
 * @returns true if queue was initialized. Returns false if queue storage
 * capacity is not sufficient for requested depth and item size
 */
bool queue_init(struct Queue * queue, uint8_t depth, uint8_t item_size);

/** Send data via the queue.
 *
 * This will copy data into queue. Queue must be initialized before
 * this call is made. The size of data is determined based on value
 * of `item_size` used during queue initialization.
 *
 * @param [in] queue pointe to queue data is being sent into
 * @param [in] data pointer to data.
 * @returns true if data was copied into queue and false if queue is
 * already full
 */

bool queue_send(struct Queue * queue, const unsigned char * data);

/** Receive data from queue.
 *
 * This will copy the oldest data out of the queue. If queue is
 * empty then this call will block until queue is filled with at
 * least one entry of data.
 *
 * @param [in] queue pointer to queue data is retrieved from
 * @param [out] data pointer to place where data will be written
 * @returns true. Returns false in case spurious interrupt occurred
 * and queue is still empty.
 */
bool queue_receive(struct Queue * queue, unsigned char * data);

/** Returns queue status.
 *
 * @param [in] queue pointer to queue queried
 * @returns true if queue is empty, false otherwise
 */
bool queue_empty(struct Queue * queue);
