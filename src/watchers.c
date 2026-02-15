#include <pthread.h>
#include <errno.h>

#include "DynamicArray/dynamic_array.h"
#include "FNotify/fnotify.h"
#include "util.h"
#include "watchers.h"
#include "threading.h"
#include "main.h"

typedef struct WMWatcher WMWatcher;

pthread_mutex_t watcher_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t exit_cond = PTHREAD_COND_INITIALIZER;
GArray watchers_array = GARRAY_STATIC_INITIALIZER(sizeof(ThreadWorkItem2));

int kill_threads = 0;
int watchers_exit = 0;

int 
WatcherInit(void)
{   
    int status;

    status = pthread_mutex_trylock(&watcher_lock);

    if(!ASSERT(status == 0))
    {   return EXIT_FAILURE;
    }

    pthread_mutex_unlock(&watcher_lock);

    status = GArrayCreateFilled(&watchers_array, sizeof(ThreadWorkItem2), 0);

    if(!ASSERT(status == EXIT_SUCCESS))
    {   return EXIT_FAILURE;
    }

    (void)ASSERT(kill_threads == 0);
    (void)ASSERT(watchers_exit == 0);

    kill_threads = 0;
    watchers_exit = 0;

    return EXIT_SUCCESS;
}

static int
WATCHER_WORK_THREAD(XCBGenericEvent *unused, Arg arg)
{
    ThreadWorkItem2 *work = arg.v;

    if(!work)
    {   return 0;
    }

    enum { EVENTS_LENGTH = 64 };
    FNotifyEvent ev[EVENTS_LENGTH];
    int events;

    Generic notify = work->arg1;
    Generic funcarg = work->arg2;
    FNotify *fnotify = notify.datav[0];

    events = FNotifyPollForEvent(fnotify, ev, EVENTS_LENGTH, 0, NULL);

    /* since cleanup is not handled by the thread since this runs on the main thread, we dont handle poll errors */
    if(events > 0)
    {   work->func(&funcarg);
    }

    return 1;
}

static void
WATCHER_THREAD(Generic *arg)
{
    enum { EVENT_LENGTH = 64 };
    enum { POLL_ERROR = -1 };
    enum { TIMEOUT_ERROR = - 2};
    enum { TIMEOUT_MS = 50 }; 

    ThreadWorkItem2 *work = arg->datav[0];

    Generic notify = work->arg1;
    Generic funcarg = work->arg2;
    FNotify *fnotify = notify.datav[0];

    FNotifyEvent events[EVENT_LENGTH];
    int eventsfilled = 0;

    int running = 1;

    while(running)
    {
        eventsfilled = FNotifyPollForEvent(fnotify, events, EVENT_LENGTH, TIMEOUT_MS, NULL);

        switch(eventsfilled)
        {
            /* poll broke exit thread */
            case POLL_ERROR:
                goto EXIT;
            case TIMEOUT_ERROR:
                break;
            default:
                if(eventsfilled >= 0)
                {   
                    _Breakpoint();
                    work->func(&funcarg);
                }
        }

        pthread_mutex_lock(&watcher_lock);

        if(kill_threads)
        {   running = 0;
        }

        pthread_mutex_unlock(&watcher_lock);
    }

EXIT:
    FNotifyDestroy(fnotify);
    free(fnotify);
    free(work);

    pthread_mutex_lock(&watcher_lock);

    if(watchers_exit > 0)
    {   --watchers_exit;
    }

    pthread_cond_broadcast(&exit_cond);

    pthread_mutex_unlock(&watcher_lock);
}

int
WatcherAdd(char *FILE_NAME, void (*func)(Generic *arg), Generic *arg, enum FNotifyFlags flags)
{
    if(!ASSERT(FILE_NAME))
    {   return EXIT_FAILURE;
    }

    if(!ASSERT(func))
    {   return EXIT_FAILURE;
    }

    if(!ASSERT(flags))
    {   return EXIT_FAILURE;
    }
    

    int status;
    FNotify *fnotify;
    ThreadWorkItem2 *work;
    Generic gen;
    Generic empty = { .data64[0] = 0 };

    fnotify = FNotifyCreate(FILE_NAME, flags);

    if(!fnotify)
    {   return EXIT_FAILURE;
    }

    work = malloc(sizeof(*work));

    if(!work)
    {
        FNotifyDestroy(fnotify);
        free(fnotify);
        return EXIT_FAILURE;
    }

    pthread_mutex_lock(&watcher_lock);

    gen.datav[0] = fnotify;

    work->arg1 = gen;
    work->arg2 = arg ? *arg : empty;
    work->func = func;

    Generic workarg = { .datav[0] = work };
    Arg workwmarg = { .v =  work };

    if(ThreadingUsesThreads())
    {
        status = ThreadingAddWork(WATCHER_THREAD, &workarg, NULL);

        if(status)
        {   goto WM_WORK;
        }
        else
        {   ++watchers_exit;
        }
    }
    else
    {
WM_WORK:
        status = GArrayPushBack(&watchers_array, work);

        /* add work if we push succesfuly */
        if(!status)
        {   status = WM_ADD_WORK(WATCHER_WORK_THREAD, workwmarg);
        }
        /* else delete it or set it to NULL if we fail */
        else
        {   
           status = GArrayDelete(&watchers_array, GArrayEnd(&watchers_array));

           if(unlikely(status))
           {    GArrayReplace(&watchers_array, NULL, GArrayEnd(&watchers_array));
           }
        }

        if(status)
        {   
            FNotifyDestroy(fnotify);
            free(fnotify);
            free(work);
        }
    }

    pthread_mutex_unlock(&watcher_lock);

    if(status)
    {   return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
WatcherDestroy(void)
{
    garray_i i;

    pthread_mutex_lock(&watcher_lock);

    for(i = GArrayStart(&watchers_array); i < GArrayEnd(&watchers_array); ++i)
    {   
        void *data = GArrayAt(&watchers_array, i);

        if(data)
        {   free(data);
        }
    }

    GArrayWipe(&watchers_array);

    kill_threads = 1;

    while(watchers_exit > 0)
    {   pthread_cond_wait(&exit_cond, &watcher_lock);
    }

    kill_threads = 0;
    watchers_exit = 0;

    pthread_mutex_unlock(&watcher_lock);
}
