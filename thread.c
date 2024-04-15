


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/signal.h>
#include <signal.h>
#include "thread.h"
/* threading Helpers */

Thread *
ThreadCreate(void *(func(void *)), void *data)
{
    Thread *thread = calloc(1, sizeof(Thread));
    if(thread)
    {
        if(!pthread_create(&thread->id, NULL, func, data))
        {
            pthread_mutex_init(&(thread->mutex), NULL);
            pthread_spin_init(&(thread->mutexq), 1);
            pthread_cond_init(&(thread->cond), NULL);
        }
        else
        {   
            free(thread);
            thread = NULL;
        }
    }
    return thread;
}

Thread *
ThreadGetSelf(void)
{
    Thread *thread = calloc(1, sizeof(Thread));

    if(thread)
    {
        thread->id = pthread_self();
        pthread_mutex_init(&(thread->mutex), NULL);
        pthread_spin_init(&(thread->mutexq), 1);
        pthread_cond_init(&(thread->cond), NULL);
    }
    return thread;
}

void
ThreadExit(void *retvalue)
{
    pthread_exit(retvalue);
}

void
ThreadKill(Thread *thread)
{
    if(thread)
    {   
        pthread_mutex_destroy(&(thread->mutex));
        pthread_spin_destroy(&(thread->mutexq));
        pthread_kill(thread->id, SIGTERM);
    }
}

void
ThreadJoin(Thread *thread)
{
    if(thread)
    {   
        pthread_join(thread->id, NULL);
    }
}

void
ThreadLock(Thread *thread)
{
    if(thread)
    {
        pthread_mutex_lock(&(thread->mutex));
    }
}

void
ThreadLockQuick(Thread *thread)
{
    if(thread)
    {
        pthread_spin_lock(&(thread->mutexq));
    }
}

void 
ThreadUnlock(Thread *thread)
{
    if(thread)
    {
        pthread_mutex_unlock(&(thread->mutex));
    }
}

void
ThreadUnlockQuick(Thread *thread)
{
    if(thread)
    {
        pthread_spin_unlock(&(thread->mutexq));
    }
}

void 
ThreadSignal(Thread *thread)
{
    if(thread)
    {
        pthread_cond_signal(&(thread->cond));
    }
}

void 
ThreadCondWait(Thread *thread, Cond *cond)
{
    if(thread)
    {
        pthread_mutex_lock(&(thread->mutex));
        pthread_cond_wait((cond), &(thread->mutex));
        pthread_mutex_unlock(&(thread->mutex));
    }
}
