#pragma once

#include "sope.h"

/**
 * Represents a queue (First In First Out)
*/
typedef struct queue {
    tlv_request_t array[QUEUE_MAX];
    int first;
    int last;
    int size;
} __attribute__((packed)) queue_t;


/**
 * @brief Inicializes a queue
 * 
 * @param queue Pointer to the queue to be inicialized
 * 
*/
void queueInitialize(queue_t * queue);

/**
 * @return returns the element at the front of the queue
*/
tlv_request_t front(queue_t queue);

/**
 * @brief Pops the element of the queue
 * 
 * @return returns true upon success, and false otherwise
*/
bool pop(queue_t * queue);

/**
 * @brief Pushes a request to the end of the queue
 * 
 * @return Returns true upon success, and false otherwise
*/
bool push(queue_t *queue, tlv_request_t request);

/**
 * @return Returns true if queue is empty, false otherwise
*/
bool isEmpty(queue_t queue);

/**
 * @return Returns true if queue is full, false otherwise
*/
bool isFull(queue_t queue);