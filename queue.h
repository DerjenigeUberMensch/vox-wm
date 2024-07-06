#ifndef _XCB_QUEUE_
#define _XCB_QUEUE_

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>


typedef struct CQueue CQueue;

struct CQueue
{
    int64_t front;
    int64_t rear;
    void **data;
    uint32_t datalen;
    size_t datasize;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/*
 * RETURN: NonZero on Full.
 * RETURN: 0 on not Full.
 */
uint8_t CQueueIsFull(CQueue *queue);
/*
 * RETURN: 1 on Empty.
 * RETURN: 0 on not Empty.
 */
uint8_t CQueueIsEmpty(CQueue *queue);
/*
 * RETURN: 1 on Success.
 * RETURN: 0 on Failure.
 */
uint8_t CQueuePop(CQueue *queue);
/*
 * RETURN: 1 on Success.
 * RETURN: 0 on Failure.
 */
uint8_t CQueueAdd(CQueue *queue, void *data);
/*
 * RETURN: void * on Success.
 * RETURN: NULL on Failure.
 */
void *CQueueGetFirst(CQueue *queue);
/*
 * RETURN: void * on Success.
 * RETURN: NULL on Failure.
 */
void *CQueueGetLast(CQueue *queue);
/*
 *
 * RETURN: 0 on Success.
 * RETURN: 1 on Failure.
 */
uint8_t CQueueCreate(void **data, uint32_t datalen, size_t size_of_one_item, CQueue *_Q_RETURN);
/* Frees any data from queue.
 */
void CQueueDestroy(CQueue *queue);


#endif
