#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#include "FNotify/fnotify.h"
#include "GArray/garray.h"
#include "util.h"
#include "watchers.h"
#include "threading.h"
#include "main.h"
#include "wmpoll.h"


typedef struct WMWatcher WMWatcher;

struct
WMWatcher
{
    FNotify *fnotify;
    void (*func)(Generic *arg);
    Generic arg;
    int fd;
};

GArray watchers_array;

int 
WatcherInit(void)
{   
    int status;

    status = GArrayCreateFilled(&watchers_array, sizeof(WMWatcher *), 0);

    if(status != EXIT_SUCCESS)
    {   return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
WATCHER_POLL_CALLBACK(int fd, int events, Generic arg)
{
    enum { EVENT_LENGTH = 64 };
    enum { POLL_ERROR = -1 };
    enum { TIMEOUT_ERROR = -2 };

    WMWatcher *watcher = arg.datav[0];

    if(!watcher)
    {   
        DebugError("Watcher is NULL");
        return;
    }

    if(!likely(events & POLLIN))
    {
        DebugWarn("No events to pollin");
        return;
    }

    FNotifyEvent notify_events[EVENT_LENGTH];

    int eventsfilled = FNotifyPollForEvent(watcher->fnotify, notify_events, EVENT_LENGTH, 0, NULL);

    switch(eventsfilled)
    {
        case POLL_ERROR:
            return;
        case TIMEOUT_ERROR:
            return;
        default:
            if(eventsfilled > 0)
            {   watcher->func(&watcher->arg);
            }

            return;
    }
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

    FNotify *fnotify;

    fnotify = FNotifyCreate(FILE_NAME, flags);

    if(!fnotify)
    {   return EXIT_FAILURE;
    }

    WMWatcher *watcher = malloc(sizeof(*watcher));

    if(!watcher)
    {
        FNotifyDestroy(fnotify);
        free(fnotify);

        return EXIT_FAILURE;
    }


    Generic empty = {0};

    watcher->fnotify = fnotify;
    watcher->func = func;
    watcher->arg = arg ? *arg : empty;
    watcher->fd = FNotifyGetFileDescriptor(fnotify);

    Generic callbackarg = { .datav[0] = watcher };


    int status;

    status = GArrayPushBack(&watchers_array, &watcher);

    if(status != EXIT_SUCCESS)
    {   
        free(watcher);
        FNotifyDestroy(fnotify);
        free(fnotify);
        return EXIT_FAILURE;
    }

    WMPollAddFD(watcher->fd, POLLIN, WATCHER_POLL_CALLBACK, callbackarg);

    return EXIT_SUCCESS;
}

void
WatcherDestroy(void)
{
    garray_i i;

    for(i = GArrayStart(&watchers_array); i < GArrayEnd(&watchers_array); ++i)
    {   
        WMWatcher **data = GArrayAt(&watchers_array, i);

        if(!data || !*data)
        {   continue;
        }

        WMWatcher *watcher = *data;

        FNotifyDestroy(watcher->fnotify);
        free(watcher->fnotify);
        free(watcher);
    }

    GArrayWipe(&watchers_array);
}
