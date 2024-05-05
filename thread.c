/* TODO FIX THREADING. */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/signal.h>
#include <signal.h>

#include "util.h"
#include "thread.h"



enum ThreadStates
{
    __THREAD_WAITING    = 1 << 0,
    __THREAD_LOCKED     = 1 << 1,
    __THREAD_LOCKEDQ    = 1 << 2,
    __THREAD_IDLE       = 1 << 3,

    __THREAD_LOCK       = 1 << 4,
    __THREAD_LOCKQ      = 1 << 5,
    __THREAD_UNLOCKQ    = 1 << 6,
    __THREAD_UNLOCK     = 1 << 7,

};

#define MUTEXLOCKED(T)      ((T)->flags & __THREAD_LOCKED)
#define SPINLOCKED(T)       ((T)->flags & __THREAD_LOCKEDQ)
#define WAITING(T)          ((T)->flags & __THREAD_WAITING)
#define LOCKED(T)           (((T)->flags & __THREAD_LOCKEDQ) | ((T)->flags & __THREAD_LOCKED))
#define IDLING(T)           ((T)->flags & __THREAD_IDLE)

#define LOCK(T)             ((T)->flags |= __THREAD_LOCK)
#define LOCKCLEAR(T)        ((T)->flags &= __THREAD_LOCK)
#define LOCKQ(T)            ((T)->flags |= __THREAD_LOCKQ)
#define LOCKQCLEAR(T)       ((T)->flags &= __THREAD_LOCKQ)

#define UNLOCK(T)           ((T)->flags |= __THREAD_UNLOCK)
#define UNLOCKCLEAR(T)      ((T)->flags &= ~__THREAD_UNLOCK)
#define UNLOCKQ(T)          ((T)->flags |= __THREAD_UNLOCKQ)
#define UNLOCKQCLEAR(T)     ((T)->flags &= ~__THREAD_UNLOCKQ)

#define MUTEXLOCK(T)        ((T)->flags |= __THREAD_LOCKED)
#define SPINLOCK(T)         ((T)->flags |= __THREAD_LOCKEDQ)
#define MUTEXUNLOCK(T)      ((T)->flags &= ~__THREAD_LOCKED)
#define SPINUNLOCK(T)       ((T)->flags &= ~__THREAD_LOCKEDQ)
#define WAIT(T)             ((T)->flags |= __THREAD_WAITING)
#define STOPWAIT(T)         ((T)->flags &= ~__THREAD_WAITING)
#define IDLE(T)             ((T)->flags |= __THREAD_IDLE)
#define STOPIDLE(T)         ((T)->flags &= ~__THREAD_IDLE)

/* threading Helpers */


static void *
_thread_runner(void *data)
{
    Thread *thread = data;
    while(thread->id)
    {
        if(thread->func)
        {   thread->func(thread->data);
        }
        else
        {
            WAIT(thread);
            ThreadCondWait(thread, thread->cond);
            STOPWAIT(thread);
        }


        if(LOCK(thread))
        {   
            pthread_mutex_lock((thread->mutex));
            MUTEXLOCK(thread);
            LOCKCLEAR(thread);
        }

        if(LOCKQ(thread))
        {   
            pthread_spin_lock((thread->mutexq));
            SPINLOCK(thread);
            LOCKQCLEAR(thread);
        }

        if(WAITING(thread))
        {   
            pthread_cond_wait(thread->cond, thread->mutex);
            STOPWAIT(thread);
        }

        if(UNLOCK(thread))
        {
            UNLOCKCLEAR(thread);
            pthread_mutex_unlock((thread->mutex));
        }
        if(UNLOCKQ(thread))
        {   
            UNLOCKQCLEAR(thread);
            pthread_spin_unlock((thread->mutexq));
        }

    }

    pthread_exit(NULL);
    return NULL;
}

static void
_exit_thread(Thread *thread)
{
    uint8_t status = ThreadLockQuick(thread);
    /* id of 0 means no thread so just die.
     * see _thread_runner.
     */
    thread->id = 0;
    if(status)
    {   ThreadUnlockQuick(thread);
    }
}

static void
_free_thread(Thread *thread)
{
    pthread_mutex_destroy((thread->mutex));
    pthread_mutex_destroy((thread->_mutex));
    pthread_spin_destroy((thread->mutexq));
    pthread_spin_destroy((thread->_mutexq));
    pthread_cond_destroy((thread->cond));

    free(thread->mutex);
    free(thread->_mutex);
    free((void *)thread->mutexq);
    free((void *)thread->_mutexq);
    free(thread->cond);
    free(thread);
}


static Handle
_get_current_handle()
{
    return pthread_self();
}

static Thread *
_init_thread()
{
    Thread *t = malloc(sizeof(Thread));

    if(t)
    {
        t->data = NULL;
        t->func = NULL;
        t->id = 0;
        t->mutex = malloc(sizeof(Mutex));
        t->_mutex = malloc(sizeof(Mutex));
        t->mutexq = malloc(sizeof(MutexQuick));
        t->_mutexq = malloc(sizeof(MutexQuick));
        t->cond = malloc(sizeof(Cond));

        if(!t->mutex || !t->_mutex || !t->mutexq || !t->_mutexq || !t->cond)
        {
            free(t->mutex);
            free(t->_mutex);
            free((void *)t->mutexq);
            free((void *)t->_mutexq);
            free(t->cond);
            free(t);
            return NULL;
        }

        pthread_mutex_init((t->mutex), NULL);
        pthread_mutex_init((t->_mutex), NULL);
        pthread_spin_init((t->mutexq), 1);
        pthread_spin_init((t->_mutexq), 1);
        pthread_cond_init((t->cond), NULL);
    }

    return t;
}

Thread *
ThreadCreate(void (func(void *)), void *data)
{
    Thread *thread = _init_thread();
    if(thread)
    {
        thread->data = data;
        thread->func = func;

        if(pthread_create(&thread->id, NULL, _thread_runner, thread))
        {   
            _free_thread(thread);
            thread = NULL;
        }
    }
    return thread;
}

Thread *
ThreadGetSelf(void)
{
    Thread *thread = _init_thread();

    if(thread)
    {
        thread->id = pthread_self();
    }
    return thread;
}

void
ThreadDetach(Thread *thread)
{
    Handle id = 0;
    if(thread)
    {   id = thread->id;
    }
    else
    {
        id = pthread_self();
    }
    pthread_detach(id);
}

void
ThreadExit(Thread *thread)
{
    if(thread)
    {  
        ThreadSignal(thread);
        _exit_thread(thread);  
        _free_thread(thread);
        thread = NULL;
    }
    else
    {
        pthread_exit(NULL);
    }
    DEBUG0("Exited Thread.");
}

void
ThreadKill(Thread *thread)
{
    if(thread)
    {   
        const Handle id = thread->id;
        ThreadExit(thread);
        pthread_kill(id, SIGTERM);
    }
    else
    {
        pthread_kill(_get_current_handle(), SIGTERM);
    }
}

void
ThreadJoin(Thread *thread)
{
    if(thread)
    {   
        const Handle id = thread->id;
        ThreadExit(thread);
        pthread_join(id, NULL);
    }
    else
    {
        pthread_join(_get_current_handle(), NULL);
    }
}

uint8_t
ThreadLock(Thread *thread)
{
    if(thread)
    {
        if(MUTEXLOCKED(thread))
        {   return 1;
        }

        if(thread->id == _get_current_handle())
        {
            pthread_mutex_lock(thread->mutex);
            MUTEXLOCK(thread);
            return 1;
        }
        LOCK(thread);
        return 1;
    }
    return 0;
}

uint8_t
ThreadLockQuick(Thread *thread)
{
    if(thread)
    {
        if(SPINLOCKED(thread))
        {   return 0;
        }
        if(thread->id == _get_current_handle())
        {
            pthread_spin_lock((thread->mutexq));
            SPINLOCK(thread);
            return 1;
        }
        LOCK(thread);
        return 1;
        
    }
    return 0;
}

void 
ThreadUnlock(Thread *thread)
{
    if(thread)
    {
        if(thread->id == _get_current_handle())
        {   
            pthread_mutex_unlock((thread->mutex));
        }
        else
        {
            UNLOCK(thread);
        }
    }
}

void
ThreadUnlockQuick(Thread *thread)
{
    if(thread)
    {
        if(thread->id == _get_current_handle())
        {   pthread_spin_unlock((thread->mutexq));
        }
        else
        {
            UNLOCKQ(thread);
        }
    }
}

void 
ThreadSignal(Thread *thread)
{
    if(thread)
    {
        pthread_cond_signal((thread->cond));
        STOPWAIT(thread);
    }
}

void 
ThreadCondWait(Thread *thread, Cond *cond)
{
    if(thread)
    {
        uint8_t status = ThreadLock(thread);
        if(thread->id == _get_current_handle())
        {   
            WAIT(thread);
            pthread_cond_wait((cond), (thread->mutex));
        }
        else
        {   WAIT(thread);
        }
        if(status)
        {   ThreadUnlock(thread);
        }
    }
}
void
ThreadChangeCaller(
        Thread *thread,
        void (*func)(void *data)
        )
{
    if(thread)
    {
        uint8_t status = ThreadLockQuick(thread);
        thread->func = func;
        if(status)
        {   ThreadUnlockQuick(thread);
        }
    }
}
        
void
ThreadChangeCallerData(
        Thread *thread,
        void *data
        )
{
    if(thread)
    {
        uint8_t status = ThreadLockQuick(thread);
        thread->data = data;
        if(status)
        {   ThreadUnlockQuick(thread);
        }
    }   
}

void
ThreadChangeFunction(
    Thread *thread,
    void (*func) (void *data),
    void *data
    )
{
    if(thread)
    {
        uint8_t signal = !thread->func;
        uint8_t status = ThreadLockQuick(thread);
        thread->func = func;
        thread->data = data;
        if(status)
        {   ThreadUnlockQuick(thread);   
        }
        if(signal)
        {   ThreadSignal(thread);
        }
    }
}

