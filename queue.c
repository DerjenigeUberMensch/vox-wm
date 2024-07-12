#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "queue.h"


uint8_t
__CQueue_full_no_lock(CQueue *queue)
{   return (queue->front == queue->rear + 1) | ((!queue->front) & (queue->rear == (queue->datalen / (queue->datasize + !queue->datasize) - 1)));
}

uint8_t 
CQueueIsFull(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    uint8_t ret = 0;
    pthread_spin_lock(&queue->spin);
    ret = __CQueue_full_no_lock(queue);
    pthread_spin_unlock(&queue->spin);
    return ret;
}

uint8_t 
CQueueIsEmpty(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    uint8_t ret = 0;
    pthread_spin_lock(&queue->spin);
    ret = queue->front == -1;
    pthread_spin_unlock(&queue->spin);
    return ret;
}

uint8_t 
CQueuePop(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    pthread_spin_lock(&queue->spin);
    if(queue->front == -1)
    {   
        pthread_spin_unlock(&queue->spin);
        return 0;
    }
    if(queue->front == queue->rear)
    {   queue->front = queue->rear = -1;
    }
    else 
    {   queue->front = (queue->front + 1) % queue->datalen;
    }
    pthread_spin_unlock(&queue->spin);
    return 1;
}

uint8_t 
CQueueAdd(CQueue *queue, void *data)
{
    if(!queue)
    {   return 0;
    }
    size_t size;
    pthread_spin_lock(&queue->spin);
    if(__CQueue_full_no_lock(queue))
    {
        pthread_spin_unlock(&queue->spin);
        return 0;
    }
    pthread_spin_unlock(&queue->spin);
    pthread_mutex_lock(&queue->mutex);

    if(queue->front == -1)
    {   queue->front = 0;
    }

    queue->rear = (queue->rear + 1) % queue->datalen;
    size = queue->datasize;
    memcpy((uint8_t *)queue->data + size * queue->rear, data, size);

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

void *
CQueueGetFirst(CQueue *queue)
{
    void *ret = NULL;
    if(!queue)
    {   return ret;
    }

    pthread_spin_lock(&queue->spin);
    if(queue->front != -1)
    {   ret = (uint8_t *)queue->data + queue->rear * queue->datasize;
    }
    pthread_spin_unlock(&queue->spin);
    return ret;
}

void *
CQueueGetLast(CQueue *queue)
{
    void *ret = NULL;
    if(!queue)
    {   return ret;
    }
    pthread_spin_lock(&queue->spin);
    if(queue->rear != -1)
    {   ret = (uint8_t *)queue->data + queue->rear * queue->datasize;
    }
    pthread_spin_unlock(&queue->spin);
    return ret;
}

uint8_t
CQueueCreate(void *data, uint32_t datalen, size_t sizeof_one_item, CQueue *_Q_RETURN)
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_spinlock_t spinlock;
    if(pthread_mutex_init(&mutex, NULL))
    {   return 1;
    }
    if(pthread_cond_init(&cond, NULL))
    {   
        pthread_mutex_destroy(&mutex);
        return 1;
    }
    if(pthread_spin_init(&spinlock, 0))
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        return 1;
    }
    CQueue queue;
    queue.data = data;
    queue.datasize = sizeof_one_item;
    queue.datalen = datalen;
    queue.rear = -1;
    queue.front = -1;
    queue.mutex = mutex;
    queue.cond = cond;
    queue.spin = spinlock;
    *_Q_RETURN = queue;
    return 0;
}

void 
CQueueDestroy(CQueue *queue)
{
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    pthread_spin_destroy(&queue->spin);
}


