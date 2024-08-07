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

extern WM _wm;

PropHandler __handler__;

/* forward declartions */
static void PropInitThreads(PropHandler *handler);
static void PropInitQueue(PropHandler *handler);


void *
Worker(
        void *x
        )
{
    PropHandler *handler = x;
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
        pthread_mutex_lock(&handler->queue.condmutex);
        pthread_cond_wait(&handler->queue.cond, &handler->queue.condmutex);
        pthread_mutex_unlock(&handler->queue.condmutex);
        /* grab if any item */
        CQueuePop(&handler->queue, &cookie);

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
CreateWorkerAttr(
        pthread_attr_t *attr
        )
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
CreateWorker(
        PropHandler *handler,
        pthread_t *id_return
        )
{
    int ret;
    pthread_attr_t attr;
    ret = CreateWorkerAttr(&attr);
    if(ret)
    {   return ret;
    }
    ret = pthread_create(id_return, &attr, Worker, handler);
    pthread_attr_destroy(&attr);
    return ret;
}

static void
PropCreateWorkers(
        PropHandler *handler,
        uint32_t workers
        )
{
    int64_t i;
    int status;
    for(i = 0; i < workers; ++i)
    {   
        status = CreateWorker(handler, &handler->threads[i]);
        /* Restart postion, if failed */
        if(status)
        {   
            Debug("Failed to create Worker thread, pthread status [%d]", status);
            --i;
            --workers;
        }
    }
}

void
PropDestroyWorkers(
        PropHandler *handler
        )
{
    while(!CQueueIsEmpty(&handler->queue))
    {   CQueuePop(&handler->queue, NULL);
    }
    while(!CQueueIsFull(&handler->queue))
    {   PropListen(handler, NULL, 0, PropExitThread);
    }
    /* Wakup any threads (if they didnt wakup already) */
    pthread_cond_broadcast(&handler->queue.cond);
    uint32_t i;
    for(i = 0; i < handler->thread_length; ++i)
    {   pthread_join(handler->threads[i], NULL);
    }
}


PropHandler *
PropCreateStatic(
        void
        )
{   return &__handler__;
}

void 
PropInit(
        PropHandler *handler
        )
{
    /* Make clean */
    memset(handler, 0, sizeof(PropHandler));
    if(!_wm.use_threads)
    {   
        handler->use_threads = 0;
        return;
    }
    
    PropInitQueue(handler);

    if(!handler->queue_data)
    {   
        handler->use_threads = 0;
        return;
    }

    u8 ret = CQueueCreate((void **)handler->queue_data, handler->queue_length, sizeof(GetPropCookie), &handler->queue);

    handler->use_threads = !ret;

    if(handler->use_threads)
    {   
        PropInitThreads(handler);
        if(handler->thread_length)
        {   PropCreateWorkers(handler, handler->thread_length);
        }
        else
        {   
            PropDestroy(handler);
            handler->use_threads = 0;
        }
    }
}

static void
PropInitThreads(
        PropHandler *handler
        )
{
    /* default just use 4 */
    u32 aloc_threads = 4;
    u32 BASE_LINE_MAX = 500;  /* Anything past 500 threads wouldnt be particularly necessary. */
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    if(cores > aloc_threads)
    {   aloc_threads = cores * 2;   /* 2 threads per core */
    }
    if(aloc_threads > BASE_LINE_MAX)
    {   aloc_threads = BASE_LINE_MAX;
    }

    void *threads = NULL;
    size_t alloc_size = aloc_threads * sizeof(*handler->threads);
    while(!threads && alloc_size)
    {   
        threads = malloc(alloc_size);
        alloc_size /= 2;
    }
    handler->thread_length = aloc_threads;
    handler->threads = threads;
}

void
PropInitQueue(
        PropHandler *handler
        )
{
    const u32 DEFAULT_SIZE = 256;
    const size_t size = sizeof(GetPropCookie) * DEFAULT_SIZE;
    handler->queue_data = malloc(size);
    handler->queue_length = DEFAULT_SIZE;
}

void
PropDestroy(
        PropHandler *handler
        )
{
    if(handler->use_threads)
    {   PropDestroyWorkers(handler);
    }
    CQueueDestroy(&handler->queue);
    free(handler->queue_data);
    free(handler->threads);
}

void 
PropListen(
        PropHandler *handler,
        XCBDisplay *display, 
        XCBWindow win, 
        enum PropertyType type
        )
{   
    PropArg arg = {0};
    PropListenArg(handler, display, win, type, arg);
}

void
PropListenArg(
        PropHandler *handler,
        XCBDisplay *display, 
        XCBWindow win, 
        enum PropertyType type, 
        PropArg arg
        )
{
    int full = CQueueIsFull(&handler->queue);
    int usethreads = handler->use_threads && _wm.use_threads;
    GetPropCookie cookie = { .win = win, .type = type, .arg = arg };
    if(usethreads && !full)
    {   CQueueAdd(&handler->queue, (void *)&cookie);
    }
    else
    {   /* single thread operation */
        PropUpdateProperty(display, &cookie);
        Debug("Using single threads, full: %d", full);
    }
}

