#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "queue.h"


static inline int
CQueueLock(CQueue *q)
{   return pthread_mutex_lock(&q->mutex);
}

static inline int
CQueueSpinLock(CQueue *q)
{   return pthread_spin_lock(&q->spin);
}

static inline int
CQueueUnlock(CQueue *q)
{   return pthread_mutex_unlock(&q->mutex);
}

static inline int
CQueueSpinUnLock(CQueue *q)
{   return pthread_spin_unlock(&q->spin);
}

static inline const int
__CQueue_Index_Exists(CQueue *q, const int64_t index)
{
    const int64_t nowrap = q->front <= q->rear;
    const int64_t wrap = !nowrap;


    const int64_t valid = index >= q->front;
    const int64_t _nowrap_ret = valid & (index < q->rear);
    const int64_t _wrap_ret = (valid & (index < q->datalen)) | ((index >= 0) & (index < q->rear));

    return (!!nowrap * _nowrap_ret) + (wrap * _wrap_ret);
}

uint8_t
__CQueue_full_no_lock(CQueue *queue)
{   
    return queue->front == queue->rear + 1 || (!queue->front && queue->rear == queue->datalen);
}

uint8_t 
CQueueIsFull(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    uint8_t ret = 0;
    CQueueSpinLock(queue);
    ret = __CQueue_full_no_lock(queue);
    CQueueSpinUnLock(queue);
    return ret;
}

uint8_t 
CQueueIsEmpty(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    uint8_t ret = 0;

    CQueueSpinLock(queue);
    ret = queue->front == -1;
    CQueueSpinUnLock(queue);
    return ret;
}

uint8_t 
CQueuePop(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    CQueueSpinLock(queue);
    if(queue->front == -1)
    {   
        CQueueSpinUnLock(queue);
        return 0;
    }
    if(queue->front == queue->rear)
    {   queue->front = queue->rear = -1;
    }
    else 
    {   queue->front = (queue->front + 1) % queue->datalen;
    }
    CQueueSpinUnLock(queue);
    return 1;
}

uint8_t 
CQueueAdd(CQueue *queue, void *data)
{
    if(!queue)
    {   return 0;
    }
    uint32_t empty;
    int64_t index;
    CQueueSpinLock(queue);
    if(__CQueue_full_no_lock(queue))
    {
        CQueueSpinUnLock(queue);
        return 0;
    }
    empty = queue->front == -1;
    index = (queue->rear + 1) % (queue->datalen);
    queue->front *= !empty;
    queue->rear = index;
    /* TODO, would be worse with spinlock if x > ~15 bytes */
    memcpy((uint8_t *)queue->data + queue->datasize * index, data, queue->datasize);
    pthread_cond_signal(&queue->cond);
    CQueueSpinUnLock(queue);
    return 1;
}

void *
CQueueGetFirst(CQueue *queue)
{
    void *ret = NULL;
    if(!queue)
    {   return ret;
    }
    CQueueSpinLock(queue);
    if(queue->front != -1)
    {   ret = (uint8_t *)queue->data + queue->rear * queue->datasize;
    }
    CQueueSpinUnLock(queue);
    return ret;
}

void *
CQueueGetLast(CQueue *queue)
{
    void *ret = NULL;
    if(!queue)
    {   return ret;
    }
    CQueueSpinLock(queue);
    if(queue->rear != -1)
    {   ret = (uint8_t *)queue->data + queue->rear * queue->datasize;
    }
    CQueueSpinUnLock(queue);
    return ret;
}

uint8_t
CQueueCreate(void *data, uint32_t datalen, size_t sizeof_one_item, CQueue *_Q_RETURN)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_spinlock_t spin;
    if(pthread_spin_init(&spin, 0))
    {   
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        return 1;
    }
    _Q_RETURN->data = data;
    _Q_RETURN->datasize = sizeof_one_item;
    _Q_RETURN->datalen = datalen;
    _Q_RETURN->rear = -1;
    _Q_RETURN->front = -1;
    _Q_RETURN->mutex = mutex;
    _Q_RETURN->cond = cond;
    _Q_RETURN->spin = spin;
    return 0;
}

void 
CQueueDestroy(CQueue *queue)
{
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    pthread_spin_destroy(&queue->spin);
}


