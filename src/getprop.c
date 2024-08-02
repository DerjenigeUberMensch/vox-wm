#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include "XCB-TRL/xcb_winutil.h"
#include "queue.h"
#include "getprop.h"
#include "main.h"

#define QUEUE_SIZE 256
/* realistically you wont ever need more than 64 as most of these threads are just waiting for data. */
#define MAX_THREADS 64

typedef struct __Property__Cookie__ __Property__Cookie__;
typedef struct ThreadHandler ThreadHandler;
typedef struct PropHandler PropHandler;

struct  
__Property__Cookie__
{
    XCBWindow win;
    enum PropertyType type;
};

struct 
ThreadHandler
{
    CQueue queue;
    uint32_t use_threads;
    pthread_t threads[MAX_THREADS];
    __Property__Cookie__ queue_data[QUEUE_SIZE];
};

struct PropHandler
{
    XCBCookie (*cookie_getter)(XCBDisplay *display, XCBWindow window);
    void (*reply_getter)(XCBDisplay *display, __Property__Cookie__ *, XCBCookie);
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

XCBCookie GetInvalidCookie(XCBDisplay *display, XCBWindow window);

static void UpdateInvalid(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateTrans(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateWindowState(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateWindowType(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateSizeHints(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateWMHints(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateWMClass(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateWMProtocol(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateStrut(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateStrutP(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateNetWMName(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateWMName(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdatePid(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateIcon(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateMotifHints(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateManage(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);
static void UpdateUnmanage(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE);

enum PropMode
{
    __PropModeCookie,
    __PropModeReply,
    __PropModeLAST,
};

static PropHandler __prop_handler[PropLAST] =
{
    [PropNone] =        { GetInvalidCookie,     UpdateInvalid     },
    [PropTransient] =   { GetTransientCookie,   UpdateTrans       },
    [PropWindowState] = { GetWindowStateCookie, UpdateWindowState },
    [PropWindowType] =  { GetWindowTypeCookie,  UpdateWindowType  },
    [PropSizeHints] =   { GetSizeHintsCookie,   UpdateSizeHints   },
    [PropWMHints] =     { GetWMHintsCookie,     UpdateWMHints     },
    [PropWMClass] =     { GetWMClassCookie,     UpdateWMClass     },
    [PropWMProtocol] =  { GetWMProtocolCookie,  UpdateWMProtocol  },
    [PropStrut] =       { GetStrutCookie,       UpdateStrut       },
    [PropStrutp] =      { GetStrutpCookie,      UpdateStrutP      },
    [PropNetWMName] =   { GetNetWMNameCookie,   UpdateNetWMName   },
    [PropWMName] =      { GetWMNameCookie,      UpdateWMName      },
    [PropPid] =         { GetPidCookie,         UpdatePid         },
    [PropIcon] =        { GetIconCookie,        UpdateIcon        },
    [PropMotifHints] =  { GetMotifHintsCookie,  UpdateMotifHints  },
    [PropManage] =      { GetInvalidCookie,     UpdateManage      },
    [PropUnmanage] =    { GetInvalidCookie,     UpdateUnmanage    },

    [PropExitThread] =  { GetInvalidCookie,     UpdateInvalid     },
};


static void
LockMainThread(void)
{   
    if(__threads.use_threads)
    {   pthread_mutex_lock(&_wm.mutex);
    }
}

static void
UnlockMainThread(void)
{   
    if(__threads.use_threads)
    {   pthread_mutex_unlock(&_wm.mutex);
    }
}

XCBCookie
GetInvalidCookie(XCBDisplay *display, XCBWindow win)
{   
    (void)display;
    (void)win;
    return (XCBCookie) { .sequence = 0 };
}

static void
UpdateInvalid(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{   
    (void)display;
    (void)cookie;
    (void)_XCB_COOKIE;
}

static void
UpdateTrans(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindow trans = 0;
    uint8_t transstatus = XCBGetTransientForHintReply(display, _XCB_COOKIE, &trans);
    if(transstatus)
    {   
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        Client *ctrans = wintoclient(trans);
        if(c)
        {   
            /* move to right desktop */
            if(ctrans && ctrans->desktop != c->desktop)
            {   setclientdesktop(c, ctrans->desktop);
            }
            /* set dialog flag(s) */
            if(!ISFLOATING(c))
            {   setfloating(c, 1);
            }
            setwtypedialog(c, 1);
        }
        UnlockMainThread();
    }
}

static void
UpdateWindowState(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, _XCB_COOKIE);
    if(prop)
    {
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {
            clientinitwstate(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdateWindowType(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, _XCB_COOKIE);
    if(prop)
    {
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   
            clientinitwtype(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdateSizeHints(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBSizeHints hints;
    int hintstatus = XCBGetWMNormalHintsReply(_wm.dpy, _XCB_COOKIE, &hints);
    if(hintstatus)
    {   
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   updatesizehints(c, &hints);
        }
        UnlockMainThread();
    }
}

static void
UpdateWMHints(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWMHints *prop = XCBGetWMHintsReply(_wm.dpy, _XCB_COOKIE);
    if(prop)
    {   
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   updatewmhints(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdateWMClass(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWMClass prop;
    int status = XCBGetWMClassReply(_wm.dpy, _XCB_COOKIE, &prop);
    if(status)
    {
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   updateclass(c, &prop);
        }
        UnlockMainThread();
        XCBWipeGetWMClass(&prop);
    }
}

static void
UpdateWMProtocol(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWMProtocols prop;
    int status = XCBGetWMProtocolsReply(_wm.dpy, _XCB_COOKIE, &prop);
    if(status)
    {   
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   updatewindowprotocol(c, &prop);
        }
        UnlockMainThread();
        XCBWipeGetWMProtocols(&prop);
    }
}

static void
UpdateStrut(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    /* TODO */
    XCBDiscardReply(display, _XCB_COOKIE);
}

static void
UpdateStrutP(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    /* TODO */
    XCBDiscardReply(display, _XCB_COOKIE);
}

static void
UpdateNetWMName(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, _XCB_COOKIE);
    char *netname = getnamefromreply(prop);
    if(prop)
    {
        LockMainThread();
        if(netname)
        {
            Client *c = wintoclient(cookie->win);
            if(c)
            {   updatetitle(c, netname, c->wmname);
            }
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdateWMName(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, _XCB_COOKIE);
    char *wmname = getnamefromreply(prop);
    if(prop)
    {
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(wmname)
        {
            if(c)
            {   updatetitle(c, c->netwmname, wmname);
            }
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdatePid(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    const int BAD_PID = -1;
    pid_t prop = XCBGetPidReply(_wm.dpy, _XCB_COOKIE);
    if(prop != BAD_PID)
    {
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   setclientpid(c, prop);
        }
        UnlockMainThread();
    }
}

static void
UpdateIcon(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, _XCB_COOKIE);
    uint32_t *icon = geticonprop(prop);
    if(prop)
    {
        LockMainThread();
        if(icon)
        {
            Client *c = wintoclient(cookie->win);
            if(c)
            {   
                free(c->icon);
                c->icon = icon;
            }
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdateMotifHints(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, _XCB_COOKIE);
    if(prop)
    {
        LockMainThread();
        Client *c = wintoclient(cookie->win);
        if(c)
        {   updatemotifhints(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
UpdateManage(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    const XCBWindow win = cookie->win;
    Client *c;
    XCBCookie requests[ManageClientLAST];
    void *replies[ManageClientLAST];

    (void)_XCB_COOKIE;

    managerequest(win, requests);
    managereplies(requests, replies);

    LockMainThread();

    c = manage(win, replies);
    if(c)
    {
        focus(c);
        arrange(c->desktop);
    }
    else if(_wm.selmon->bar && _wm.selmon->bar->win == win)
    {
        focus(NULL);
        arrange(_wm.selmon->desksel);
    }
    XCBFlush(_wm.dpy);
    UnlockMainThread();
}

static void
UpdateUnmanage(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    XCBWindow win = cookie->win;
    Client *c;
    Desktop *desk;
    (void)display;
    (void)_XCB_COOKIE;
    LockMainThread();

    c = wintoclient(win);
    if(c)
    {   
        desk = c->desktop;
        unmanage(c, 0);
        if(desk->mon->desksel == desk)
        {   
            focus(NULL);
            arrange(desk);
        }
        XCBFlush(_wm.dpy);
    }

    UnlockMainThread();
}

static void
UpdateProperty(XCBDisplay *display, __Property__Cookie__ *cookie)
{
    /* get the cookie */
    const XCBCookie _XCB_COOKIE = __prop_handler[cookie->type].cookie_getter(display, cookie->win);
    /* get the reply and update prop */
    __prop_handler[cookie->type].reply_getter(display, cookie, _XCB_COOKIE);
}

void *
Worker(void *x)
{
    int screen;
    XCBDisplay *display = XCBOpenDisplay(NULL, &screen);
    __Property__Cookie__ cookie = { .win = 0, .type = 0 };
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
            UpdateProperty(display, &cookie);
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
    uint8_t ret = CQueueCreate((void **)&__threads.queue_data, QUEUE_SIZE, sizeof(__Property__Cookie__), &__threads.queue);
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
    int full = CQueueIsFull(&__threads.queue);
    int usethreads = __threads.use_threads && _wm.use_threads;
    __Property__Cookie__ cookie;
    cookie.win = win;
    cookie.type = type;
    if(usethreads && !full)
    {   CQueueAdd(&__threads.queue, (void *)&cookie);
    }
    else
    {   /* single thread operation */
        UpdateProperty(display, &cookie);
        Debug("Using single threads, full: %d", full);
    }
}
