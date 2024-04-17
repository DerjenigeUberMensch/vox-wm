#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


uint8_t 
CQueueIsFull(CQueue *queue)
{
    if(queue)
    {   return (queue->front == queue->rear + 1) | ((!queue->front) & (queue->rear == (queue->datalen / (queue->datasize + !queue->datasize) - 1)));
    }
    return 0;
}

uint8_t 
CQueueIsEmpty(CQueue *queue)
{
    if(queue)
    {   return (queue->front == -1);
    }
    return 0;
}

uint8_t 
CQueuePop(CQueue *queue)
{
    if(queue)
    {
        if(queue->front == -1)
        {   return 0;
        }
        if(queue->front == queue->rear)
        {   queue->front = queue->rear = -1;
        }
        else 
        {   queue->front = (queue->front + 1) % queue->datalen;
        }
        return 1;
    }
    return 0;
}

uint8_t 
CQueueAdd(CQueue *queue, void *data)
{
    if(queue)
    {
        if(CQueueIsFull(queue))
        {   return 0;
        }
        
        if(queue->front == -1)
        {   queue->front = 0;
        }

        queue->rear = (queue->rear + 1) % queue->datalen;
        queue->data[queue->rear] = data;
        return 1;
    }
    return 0;
}

void *
CQueueGetFirst(CQueue *queue)
{
    if(queue)
    {
        if(queue->front == -1)
        {   return NULL;
        }
        return queue->data[queue->front];
    }
    return NULL;
}

void *
CQueueGetLast(CQueue *queue)
{
    if(queue)
    {
        if(queue->rear == -1)
        {   return NULL;
        }
        return queue->data[queue->rear];
    }
    return NULL;
}

CQueue *
CQueueCreate(void **data, uint32_t datalen, size_t sizeof_one_item)
{
    CQueue *queue = malloc(sizeof(CQueue));

    if(queue)
    {
        queue->data = data;
        queue->datasize = sizeof_one_item;
        queue->rear = -1;
        queue->front = -1;
    }

    return queue;
}
