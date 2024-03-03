#ifndef QUEUE_H
#define QUEUE_H


#include <stdint.h>

typedef struct QueueItem QueueItem;
typedef struct CircularQueue CircularQueue;

struct QueueItem
{
    void *cookie;
    void *data;
};

struct CircularQueue
{
    int32_t  front;
    int32_t  rear;
    int32_t  size;
    QueueItem items[];
};
/* Creates a new CircularQueue.
 * 
 * item_count:              The count to the max number of possible QueueItem's in the CircularQueue structure.
 *
 * NOTE: int32_t specifier has NO bound checks for negative numbers however DOES still have the regular int32_t number limit.
 *
 * RETURN: CircularQueue * On Success.
 * RETURN: NULL On Failure.
 */
CircularQueue *CircularQueueCreate(int32_t item_count);
/* Checks if the queue is Full.
 *
 * RETURN: 1 On Full.
 * RETURN: 0 On Not Full.
 */
int CircularQueueFull(CircularQueue *queue);
/* Checks if the queue is Empty.
 *
 * RETURN: 1 On Empty.
 * RETURN: 0 On Not Empty.
 */
int CircularQueueEmpty(CircularQueue *queue);
/* Adds a QueueItem to the queue.
 *
 * RETURN: 0 On Failure.
 * RETURN: 1 On Success.
 */
int CircularQueueAdd(CircularQueue *queue, QueueItem *item);
/* Removes a item from the queue.
 * 
 * NOTE: Item should NOT be freed as it is simply the adress of the queue array item.
 *
 * RETURN: NULL On Failure.
 * RETURN: QueueItem * to the item removed.
 */
QueueItem *CircularQueueRemove(CircularQueue *queue, QueueItem *item);









#endif
