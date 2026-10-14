#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "queue.h"

static inline int
CQueueLockR(CQueue *q)
{   return pthread_rwlock_rdlock(&q->mutex);
}

static inline int
CQueueLockW(CQueue *q)
{   return pthread_rwlock_wrlock(&q->mutex);
}

static inline int
CQueueUnlockR(CQueue *q)
{   return pthread_rwlock_unlock(&q->mutex);
}

static inline int
CQueueUnlockW(CQueue *q)
{   return pthread_rwlock_unlock(&q->mutex);
}

/*
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

*/

uint8_t
__CQueue_full_no_lock(CQueue *queue)
{   
    return queue->front == queue->rear + 1 || (!queue->front && queue->rear == queue->datalen - 1);
}

uint8_t 
CQueueIsFull(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    uint8_t ret = 0;
    CQueueLockR(queue);
    ret = __CQueue_full_no_lock(queue);
    CQueueUnlockR(queue);
    return ret;
}

uint8_t 
CQueueIsEmpty(CQueue *queue)
{
    if(!queue)
    {   return 0;
    }
    uint8_t ret = 0;

    CQueueLockR(queue);
    ret = queue->front == -1;
    CQueueUnlockR(queue);
    return ret;
}

uint8_t 
CQueuePop(CQueue *queue, void *fill)
{
    if(!queue)
    {   return 0;
    }
    CQueueLockW(queue);
    if(queue->front == -1)
    {   
        CQueueUnlockW(queue);
        return 0;
    }
    if(fill)
    {   memcpy(fill, (uint8_t *)queue->data + (queue->datasize * queue->front), queue->datasize);
    }
    if(queue->front == queue->rear)
    {   queue->front = queue->rear = -1;
    }
    else 
    {   queue->front = (queue->front + 1) % queue->datalen;
    }
    CQueueUnlockW(queue);
    return 1;
}

uint8_t 
CQueueAdd(CQueue *queue, void *data)
{
    if(!queue || !data)
    {   return 0;
    }

    uint32_t empty;
    int64_t index;
    CQueueLockW(queue);
    if(__CQueue_full_no_lock(queue))
    {
        CQueueUnlockW(queue);
        return 0;
    }
    empty = queue->front == -1;
    index = (queue->rear + 1) % (queue->datalen);
    queue->front *= !empty;
    queue->rear = index;

    memcpy((uint8_t *)queue->data + queue->datasize * index, data, queue->datasize);
    pthread_mutex_lock(&queue->condmutex);
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->condmutex);
    CQueueUnlockW(queue);
    return 1;
}

void *
CQueueGetFirst(CQueue *queue)
{
    void *ret = NULL;
    
    if(!queue)
    {   return ret;
    }

    CQueueLockR(queue);

    if(queue->front != -1)
    {   ret = (uint8_t *)queue->data + queue->front * queue->datasize;
    }

    CQueueUnlockR(queue);
    return ret;
}

void *
CQueueGetLast(CQueue *queue)
{
    void *ret = NULL;
    if(!queue)
    {   return ret;
    }
    CQueueLockR(queue);
    if(queue->rear != -1)
    {   ret = (uint8_t *)queue->data + queue->rear * queue->datasize;
    }
    CQueueUnlockR(queue);
    return ret;
}

uint8_t
CQueueCreate(uint32_t datalen, size_t sizeof_one_item, CQueue *_Q_RETURN)
{
    if(!_Q_RETURN || !datalen || !sizeof_one_item || sizeof_one_item > SIZE_MAX / datalen)
    {   return 1;
    }

    memset(_Q_RETURN, 0, sizeof(*_Q_RETURN));

    _Q_RETURN->data = malloc(sizeof_one_item * datalen);

    if(!_Q_RETURN->data)
    {   return 1;
    }

    _Q_RETURN->datasize = sizeof_one_item;
    _Q_RETURN->datalen = datalen;
    _Q_RETURN->rear = -1;
    _Q_RETURN->front = -1;

    if(pthread_rwlock_init(&_Q_RETURN->mutex, NULL))
    {   goto FREE_DATA;
    }

    if(pthread_mutex_init(&_Q_RETURN->condmutex, NULL))
    {   goto FREE_RWLOCK;
    }

    if(pthread_cond_init(&_Q_RETURN->cond, NULL))
    {   goto FREE_MUTEX;
    }

    return 0;
FREE_MUTEX:
    pthread_mutex_destroy(&_Q_RETURN->condmutex);
FREE_RWLOCK:
    pthread_rwlock_destroy(&_Q_RETURN->mutex);
FREE_DATA:
    free(_Q_RETURN->data);

    _Q_RETURN->data = NULL;

    return 1;
}

void 
CQueueDestroy(CQueue *queue)
{
    free(queue->data);
    pthread_rwlock_destroy(&queue->mutex);
    pthread_mutex_destroy(&queue->condmutex);
    pthread_cond_destroy(&queue->cond);
}


