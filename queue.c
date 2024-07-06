#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "queue.h"


uint8_t 
CQueueIsFull(CQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    if(queue)
    {   return (queue->front == queue->rear + 1) | ((!queue->front) & (queue->rear == (queue->datalen / (queue->datasize + !queue->datasize) - 1)));
    }
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

uint8_t 
CQueueIsEmpty(CQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    if(queue)
    {   return (queue->front == -1);
    }
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

uint8_t 
CQueuePop(CQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);
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
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

uint8_t 
CQueueAdd(CQueue *queue, void *data)
{
    pthread_mutex_lock(&queue->mutex);
    if(queue)
    {
        if(CQueueIsFull(queue))
        {   return 0;
        }
        
        if(queue->front == -1)
        {   queue->front = 0;
        }

        queue->rear = (queue->rear + 1) % queue->datalen;
        memcpy(queue->data[queue->rear], data, queue->datasize);
        return 1;
    }
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

void *
CQueueGetFirst(CQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    if(queue)
    {
        if(queue->front == -1)
        {   return NULL;
        }
        return queue->data[queue->front];
    }
    pthread_mutex_unlock(&queue->mutex);
    return NULL;
}

void *
CQueueGetLast(CQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    if(queue)
    {
        if(queue->rear == -1)
        {   return NULL;
        }
        return queue->data[queue->rear];
    }
    pthread_mutex_unlock(&queue->mutex);
    return NULL;
}

uint8_t
CQueueCreate(void **data, uint32_t datalen, size_t sizeof_one_item, CQueue *_Q_RETURN)
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    if(pthread_mutex_init(&mutex, NULL))
    {   return 1;
    }
    if(pthread_cond_init(&cond, NULL))
    {   
        pthread_mutex_destroy(&mutex);
        return 1;
    }
    CQueue queue;
    queue.data = data;
    queue.datasize = sizeof_one_item;
    queue.rear = -1;
    queue.front = -1;
    queue.mutex = mutex;
    queue.cond = cond;
    *_Q_RETURN = queue;
    return 0;
}

void 
CQueueDestroy(CQueue *queue)
{
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}


