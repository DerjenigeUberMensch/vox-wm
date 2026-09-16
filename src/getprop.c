#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "TPromises/tpromise.h"
#include "XCB-TRL/xcb_trl.h"
#include "XCB-TRL/xcb_winutil.h"
#include "queue.h"
#include "getprop.h"
#include "prop.h"
#include "main.h"
#include "threading.h"
#include "util.h"

extern WM _wm;

static CQueue _queue;

static sem_t counter;

static TPromise _worked_exited;

static int intialized = 0;

void
Worker(
        Generic *x
        )
{
    GetPropCookie cookie;
    int status;

    (void)x;

    while(true)
    {
        /* signal interupt */
        int signo = EINTR;
        /* invalid sempahor */
        int inval = EINVAL;

        while(true)
        {
            errno = 0;

            status = sem_wait(&counter);

            if(status == -1 && (errno == signo))
            {   continue;
            }
            else if(status == -1)
            {   
                if(unlikely(errno == inval))
                {   DebugError("Semaphore is invalid");
                }
                else
                {   DebugError("Failed to wait on semaphore");
                }

                ResolveTPromise(&_worked_exited, NULL);
                return;
            }
            else
            {   break;
            }
        }

        if(unlikely(CQueueIsEmpty(&_queue)))
        {
            DebugError("Queue is desynced");
            continue;
        }

        status = CQueuePop(&_queue, &cookie);

        if(!status)
        {   
            DebugError("Failed to pop from queue");
            continue;
        }

        if(cookie.type == PropExitThread)
        {   
            Debug("Worker thread exiting");
            ResolveTPromise(&_worked_exited, NULL);
            return;
        }

        /* get replires back before handing it */
        XCBFlush(_wm.dpy);

        PropUpdatePropertyGetReply(_wm.dpy, &cookie);
    }
}

void
QueueProperty(XCBDisplay *display, GetPropCookie *cookie)
{
    if(!intialized)
    {   return;
    }

    if(!ASSERT(cookie && display))
    {   return;
    }

    int status;

    if(CQueueIsFull(&_queue))
    {
        if(cookie->type == PropExitThread)
        {   
            /* discard events were shuttindg down Cookies go down with the display */
            while(CQueueIsFull(&_queue))
            {   CQueuePop(&_queue, NULL);
            }

            status = CQueueAdd(&_queue, cookie);

            /* arbitrary number */
            const int MAX_TILL_IGNORE = 25;

            int cnt = 0;

            while(unlikely(!status) && cnt < MAX_TILL_IGNORE)
            {   
                DebugWarn("Failed to add property to queue, retrying");
                sched_yield();
                status = CQueueAdd(&_queue, cookie);
                ++cnt;
            }

            /* take the memory leak if the other trhead doesnt want to shut down */
            if(!status && cnt >= MAX_TILL_IGNORE)
            {   
                DebugError("Failed to add property to queue, ignoring");

                /* abort on debug builds */
                (void)ASSERT(0);
                return;
            }

            goto POST;
        }
        else
        {
            DebugWarn("Using single threads: OUT_OF_QUEUE_MEMORY");

            PropUpdatePropertyGetReply(display, cookie);
            return;
        }
    }

    status = CQueueAdd(&_queue, cookie);

    if(!status)
    {   
        DebugWarn("Failed to add property to queue");
        return;
    }
POST:
    status = sem_post(&counter);

    /* were fucked */
    if(unlikely(status == -1))
    {   
        DebugWarn("Failed to post semaphore");
    }
}

void
PropInit(void)
{
    if(!ASSERT(!intialized))
    {
        DebugError("Prop already intialized");
        return;
    }

    if(!_wm.use_threads)
    {   
        DebugWarnOnce("Using single threads: NO_MULTI_THREAD");
        return;
    }

    /* queue size doesnt matter just needs to be big enough */
    enum { QUEUE_LEN = 1024 * 3 };

    int status;
    size_t itemsize = sizeof(GetPropCookie);

    status = CQueueCreate(QUEUE_LEN, itemsize, &_queue);

    if(status)
    {
        Debug("Failed to create queue");
        return;
    }

    status = NewTPromiseFilled(&_worked_exited);

    if(status == EXIT_FAILURE)
    {
        Debug("Failed to create promise");
        CQueueDestroy(&_queue);
        return;
    }

    status = sem_init(&counter, 0, 0);

    if(status == -1)
    {
        DebugError("Failed to create semaphore");

        CQueueDestroy(&_queue);
        DestroyTPromiseFilled(&_worked_exited);
        return;
    }

    status = ThreadingAddWork(Worker, NULL, NULL);

    if(status == EXIT_FAILURE)
    {
        Debug("Failed to create worker thread");
        CQueueDestroy(&_queue);
        sem_destroy(&counter);
        DestroyTPromiseFilled(&_worked_exited);
        return;
    }

    intialized = 1;
}

void
PropDestroy(void)
{
    if(!intialized)
    {   return;
    }

    if(!ASSERT(_wm.use_threads))
    {   
        DebugError("UB encountered");
        /* leak memory over crashing */
        return;
    }

    PropListen(_wm.dpy, 0, PropExitThread);

    int MAX_TIME_WAIT_SECONDS = 2;
    double MAX_TIME_WAIT_MILLISECONDS = MAX_TIME_WAIT_SECONDS * 1000.0;

    AwaitTPromiseTimeout(&_worked_exited, MAX_TIME_WAIT_MILLISECONDS);

    CQueueDestroy(&_queue);
    sem_destroy(&counter);

    if(IsTPromiseDone(&_worked_exited))
    {   DestroyTPromiseFilled(&_worked_exited);
    }
    else
    {   DebugError("Worker thread failed to exit: MEM_LEAK_WARN");
    }

    intialized = 0;
}

void 
PropListen(
        XCBDisplay *display, 
        XCBWindow win, 
        enum PropertyType type
        )
{   
    PropArg arg = {0};
    PropListenArg(display, win, type, arg);
}

void
PropListenArg(
        XCBDisplay *display, 
        XCBWindow win, 
        enum PropertyType type, 
        PropArg arg
        )
{
    int usethreads;

    LOCK_WM();
    usethreads = _wm.use_threads;
    UNLOCK_WM();

    GetPropCookie cookie = { .win = win, .type = type, .arg = arg};

    if(type == PropExitThread)
    {   
        if(usethreads && intialized)
        {   QueueProperty(display, &cookie);
        }
        return;
    }

    if(!usethreads || !intialized)
    {   
        DebugWarnOnce("Using single threads: NO_MULTI_THREAD");
        PropUpdateProperty(display, &cookie);
        return;
    }

    PropUpdatePropertyGetCookie(display, &cookie);
    QueueProperty(display, &cookie);
}

