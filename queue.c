
#include <stdlib.h>

#include "queue.h"


CircularQueue *
CircularQueueCreate(int32_t item_count)
{
    CircularQueue *queue = malloc(sizeof(CircularQueue) + (sizeof(QueueItem) * item_count));

    if(!queue)
    {   return NULL;
    }
    queue->size = item_count;
    queue->front = -1;
    queue->rear = -1;
    return queue;
}

void
CircularQueueDestroy(CircularQueue *queue)
{
    free(queue);
}

int 
CircularQueueFull(CircularQueue *queue)
{                                                           /* && often uses a conditional jmp */
    return (queue->front == queue->rear + 1) || ((queue->front == 0) & (queue->rear == queue->size - 1));
}

int 
CircularQueueEmpty(CircularQueue *queue)
{
    return queue->front == -1;
}
int 
CircularQueueAdd(CircularQueue *queue, QueueItem *item)
{
    if(CircularQueueFull(queue))
    {   return 0;
    }
    if(queue->front == -1)
    {   queue->front = 0;
    }
    queue->rear = (queue->rear + 1) % queue->size;
    queue->items[queue->rear] = *item;
    return 1;
}

QueueItem *
CircularQueueRemove(CircularQueue *queue, QueueItem *item)
{
    if(CircularQueueEmpty(queue))
    {   return NULL;
    }

    QueueItem *ret;

    ret = &queue->items[queue->front];

    if(queue->front == queue->rear)
    {   queue->front = queue->rear = -1;
    }
    else 
    {   queue->front = (queue->front + 1) % queue->size;
    }
    return ret;
}
