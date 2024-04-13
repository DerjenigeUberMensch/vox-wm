#ifndef THREADING_H_
#define THREADING_H_

#include <pthread.h>


typedef struct Thread Thread;
typedef pthread_t Handle;
typedef pthread_mutex_t Mutex;
typedef pthread_cond_t Cond;
typedef pthread_spinlock_t MutexQuick;


struct Thread
{
    Handle id;
    Mutex mutex;
    Cond cond;
    MutexQuick mutexq;
};


Thread *
ThreadCreate(
        void *func(void *), 
        void *data
        );

/* Must be freed.
 */
Thread *
ThreadGetSelf(
        void
        );

void 
ThreadExit(
        Thread *thread
        );

void 
ThreadJoin(
        Thread *thread
        );

void 
ThreadLock(
        Thread *thread
        );
void
ThreadLockQuick(
        Thread *thread
        );

void 
ThreadUnlock(
        Thread *thread
        );
void
ThreadUnlockQuick(
        Thread *thread
        );

void 
ThreadSignal(
        Thread *thread
        );

void 
ThreadCondWait(
        Thread *thread, 
        Cond *cond
        );


#endif
