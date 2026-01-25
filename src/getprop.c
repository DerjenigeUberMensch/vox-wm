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
#include "threading.h"

extern WM _wm;


void
Worker(
        Generic *x
        )
{
    GetPropCookie *cookie = (GetPropCookie *)x->datav[0];

    LOCK_WM();
    PropUpdateProperty(_wm.dpy, cookie);
    UNLOCK_WM();

    free(cookie);
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
    int status;

    LOCK_WM();
    usethreads = _wm.use_threads;
    UNLOCK_WM();

    if(type == PropExitThread)
    { 	return;
    }

    GetPropCookie cookie = { .win = win, .type = type, .arg = arg };
    Generic data = {0};

    if(!usethreads)
    {   
        Debug0("Using single threads: NO_MULTI_THREAD");
        goto SINGLE_THREAD;
    }

    data.datav[0] = malloc(sizeof(GetPropCookie));

    if(!data.datav[0])
    {   
        Debug0("Using single threads: OUT_OF_MEMORY");
        goto SINGLE_THREAD;
    }
    else
    {
        *(GetPropCookie *)data.datav[0] = cookie;
        status = ThreadingAddWork(Worker, &data, NULL);

        /* TODO MEMORY LEAK? */
        if(status == EXIT_FAILURE)
        {   
            free(data.datav[0]);
            goto SINGLE_THREAD;
        }
    }

    return;
SINGLE_THREAD:
    PropUpdateProperty(display, &cookie);
}

