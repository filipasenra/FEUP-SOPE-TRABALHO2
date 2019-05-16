#include "queue.h"

void queueInitialize(queue_t *queue)
{
    queue->first = 0;
    queue->last = 0;
    queue->size = 0;
}

tlv_request_t front(queue_t queue)
{
    return queue.array[queue.first];
}

bool pop(queue_t *queue)
{
    if (isEmpty(*queue))
        return false;

    queue->first = (queue->first + 1) % QUEUE_MAX;
    queue->size--;

    return true;
}

bool push(queue_t *queue, tlv_request_t request)
{
    if (isFull(*queue))
        return false;

    queue->array[queue->last] = request;
    queue->last = (queue->last + 1) % QUEUE_MAX;
    queue->size++;

    return true;
}

bool isEmpty(queue_t queue)
{
    return (queue.size == 0);
}

bool isFull(queue_t queue)
{
    return (queue.size == QUEUE_MAX);
}