#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include "XCB-TRL/xcb_winutil.h"
#include "queue.h"
#include "getprop.h"
#include "prop.h"
#include "main.h"

#define QUEUE_SIZE 256
/* realistically you wont ever need more than 64 as most of these threads are just waiting for data. */
#define MAX_THREADS 64

typedef struct ThreadHandler ThreadHandler;


struct 
ThreadHandler
{
    CQueue queue;
    uint32_t use_threads;
    pthread_t threads[MAX_THREADS];
    GetPropCookie queue_data[QUEUE_SIZE];
};

static ThreadHandler __threads;

extern WM _wm;
/* These dont require mutex for the following reasons:
 * - They are stack allocated.
 * - They are set during setup() before propertynotify is intialized.
 * - They are never changed afterwards.
 * - These threads are killed before exit, to prevent the stack from de-initializing these variables.
 */
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];
extern XCBAtom motifatom;

void *
Worker(void *x)
{
    int screen;
    XCBDisplay *display = XCBOpenDisplay(NULL, &screen);
    GetPropCookie cookie = { .win = 0, .type = 0 };
    if(!display)
    {   
        Debug0("Failed to open display for worker");
        return NULL;
    }
    while(cookie.type != PropExitThread)
    {
        /* wait for stuff to happen */
        pthread_mutex_lock(&__threads.queue.condmutex);
        pthread_cond_wait(&__threads.queue.cond, &__threads.queue.condmutex);
        pthread_mutex_unlock(&__threads.queue.condmutex);
        /* grab if any item */
        CQueuePop(&__threads.queue, &cookie);

        if(cookie.win)
        {   
            PropUpdateProperty(display, &cookie);
            cookie.win = 0;
        }
    }
    XCBCloseDisplay(display);
    return NULL;
}


static int
CreateWorkerAttr(pthread_attr_t *attr)
{
    int status = 0;
    status = pthread_attr_init(attr);
    if(status)
    {   return status;
    }
    status = pthread_attr_setstacksize(attr, PTHREAD_STACK_MIN);
    if(status)
    {   
        pthread_attr_destroy(attr);
        return status;
    }
    status = pthread_attr_setguardsize(attr, 0);
    if(status)
    {
        pthread_attr_destroy(attr);
        return status;
    }
    return 0;
}

/*
 * RETURN: pthread_create() return values.
 */
static int
CreateWorker(pthread_t *id_return)
{
    int ret;
    pthread_attr_t attr;
    ret = CreateWorkerAttr(&attr);
    if(ret)
    {   return ret;
    }
    ret = pthread_create(id_return, &attr, Worker, NULL);
    pthread_attr_destroy(&attr);
    return ret;
}

static uint32_t
PropGetThreadCount(void)
{
    static uint32_t threads = 0;
    if(threads)
    {   return threads;
    }
    /* default just use 4 */
    uint32_t aloc_threads = 4;
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    if(cores > aloc_threads)
    {   aloc_threads = cores * 2;   /* 2 threads per core */
    }
    if(aloc_threads > MAX_THREADS)
    {   aloc_threads = MAX_THREADS;
    }
    threads = aloc_threads;
    return aloc_threads;
}

static void
PropCreateWorkers(uint32_t threads)
{
    int32_t i;
    for(i = 0; i < threads; ++i)
    {   CreateWorker(&__threads.threads[i]);
    }
}

static void
PropDestroyWorkers(uint32_t threads)
{
    while(!CQueueIsEmpty(&__threads.queue))
    {   CQueuePop(&__threads.queue, NULL);
    }
    while(!CQueueIsFull(&__threads.queue))
    {   PropListen(NULL, 0, PropExitThread);
    }
    /* Wakup any threads (if they didnt wakup already) */
    pthread_cond_broadcast(&__threads.queue.cond);
    uint32_t i;
    for(i = 0; i < threads; ++i)
    {   pthread_join(__threads.threads[i], NULL);
    }
}

void 
PropInit(void)
{
    /* Make clean */
    memset(&__threads, 0, sizeof(ThreadHandler));
    if(!_wm.use_threads)
    {   
        __threads.use_threads = 0;
        return;
    }
    uint8_t ret = CQueueCreate((void **)&__threads.queue_data, QUEUE_SIZE, sizeof(GetPropCookie), &__threads.queue);
    __threads.use_threads = !ret;
    if(!ret)
    {   PropCreateWorkers(PropGetThreadCount());
    }
}

void
PropDestroy(void)
{
    if(!_wm.use_threads)
    {   return;
    }
    if(__threads.use_threads)
    {   PropDestroyWorkers(PropGetThreadCount());
    }
    CQueueDestroy(&__threads.queue);
}

void 
PropListen(XCBDisplay *display, XCBWindow win, enum PropertyType type)
{   
    PropArg arg = {0};
    PropListenArg(display, win, type, arg);
}

void
PropListenArg(XCBDisplay *display, XCBWindow win, enum PropertyType type, PropArg arg)
{
    int full = CQueueIsFull(&__threads.queue);
    int usethreads = __threads.use_threads && _wm.use_threads;
    GetPropCookie cookie;
    cookie.win = win;
    cookie.type = type;
    cookie.arg = arg;
    if(usethreads && !full)
    {   CQueueAdd(&__threads.queue, (void *)&cookie);
    }
    else
    {   /* single thread operation */
        PropUpdateProperty(display, &cookie);
        Debug("Using single threads, full: %d", full);
    }
}



