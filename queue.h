#pragma once

#include "sope.h"

typedef struct queue {
    tlv_request_t array[QUEUE_MAX];
    int first;
    int last;
    int size;
} __attribute__((packed)) queue_t;

void queueInicialize(queue_t * queue);

tlv_request_t front(queue_t queue);

bool pop(queue_t * queue);

bool push(queue_t *queue, tlv_request_t request);

bool isEmpty(queue_t queue);

bool isFull(queue_t queue);