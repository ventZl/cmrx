#pragma once

#include <stdbool.h>
#include <stdint.h>

#define QUEUE_LENGTH    256

struct Queue {
    unsigned char content[QUEUE_LENGTH];
    volatile uint8_t write_cursor;
    volatile uint8_t read_cursor;
    volatile bool empty;
    uint8_t depth;
    uint8_t item_size;

};

bool queue_send(struct Queue * queue, const unsigned char * data);
bool queue_receive(struct Queue * queue, unsigned char * data);
bool queue_empty(struct Queue * queue);
bool queue_init(struct Queue * queue, uint8_t depth, uint8_t item_size);
