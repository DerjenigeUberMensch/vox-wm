#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include "XCB-TRL/xcb_winutil.h"
#include "queue.h"
#include "getprop.h"
#include "dwm.h"

#define QUEUE_SIZE 1024
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
    XCBCookie (*cookie_getter)(XCBDisplay *, XCBWindow);
    void (*reply_getter)(XCBDisplay *, __Property__Cookie__ *, XCBCookie);
};

static ThreadHandler __threads;

/* These dont require mutex for the following reasons:
 * - They are stack allocated.
 * - They are set during setup() before propertynotify is intialized.
 * - They are never changed afterwards.
 * - These threads are killed before exit, to prevent the stack from de-initializing these variables.
 */
extern WM _wm;
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];
extern XCBAtom motifatom;

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

static XCBCookie GetInvalid(XCBDisplay *, XCBWindow);
static XCBCookie GetTransient(XCBDisplay *, XCBWindow);
static XCBCookie GetWindowState(XCBDisplay *, XCBWindow);
static XCBCookie GetWindowType(XCBDisplay *, XCBWindow);
static XCBCookie GetSizeHints(XCBDisplay *, XCBWindow);
static XCBCookie GetWMHints(XCBDisplay *, XCBWindow);
static XCBCookie GetWMClass(XCBDisplay *, XCBWindow);
static XCBCookie GetWMProtocol(XCBDisplay *, XCBWindow);
static XCBCookie GetStrut(XCBDisplay *, XCBWindow);
static XCBCookie GetStrutp(XCBDisplay *, XCBWindow);
static XCBCookie GetNetWMName(XCBDisplay *, XCBWindow);
static XCBCookie GetWMName(XCBDisplay *, XCBWindow);
static XCBCookie GetPid(XCBDisplay *, XCBWindow);
static XCBCookie GetIcon(XCBDisplay *, XCBWindow);
static XCBCookie GetMotifHints(XCBDisplay *, XCBWindow);
enum PropMode
{
    __PropModeCookie,
    __PropModeReply,
    __PropModeLAST,
};

static PropHandler __prop_handler[PropLAST] =
{
    [PropNone] =        { GetInvalid,     UpdateInvalid     },
    [PropTransient] =   { GetTransient,   UpdateTrans       },
    [PropWindowState] = { GetWindowState, UpdateWindowState },
    [PropWindowType] =  { GetWindowType,  UpdateWindowType  },
    [PropSizeHints] =   { GetSizeHints,   UpdateSizeHints   },
    [PropWMHints] =     { GetWMHints,     UpdateWMHints     },
    [PropWMClass] =     { GetWMClass,     UpdateWMClass     },
    [PropWMProtocol] =  { GetWMProtocol,  UpdateWMProtocol  },
    [PropStrut] =       { GetStrut,       UpdateStrut       },
    [PropStrutp] =      { GetStrutp,      UpdateStrutP      },
    [PropNetWMName] =   { GetNetWMName,   UpdateNetWMName   },
    [PropWMName] =      { GetWMName,      UpdateWMName      },
    [PropPid] =         { GetPid,         UpdatePid         },
    [PropIcon] =        { GetIcon,        UpdateIcon        },
    [PropMotifHints] =  { GetMotifHints,  UpdateMotifHints  },
    [PropExitThread] =  { GetInvalid,     UpdateInvalid     },
};


static void
LockMainThread()
{   
    if(__threads.use_threads)
    {   pthread_mutex_lock(&_wm.mutex);
    }
}

static void
UnlockMainThread()
{   
    if(__threads.use_threads)
    {   pthread_mutex_unlock(&_wm.mutex);
    }
}

static XCBCookie
GetInvalid(XCBDisplay *display, XCBWindow win)
{   
    (void)display;
    (void)win;
    return (XCBCookie) { .sequence = 0 };
}

static XCBCookie
GetTransient(XCBDisplay *display, XCBWindow win)
{   return XCBGetTransientForHintCookie(display, win);
}

static XCBCookie
GetWindowState(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

static XCBCookie
GetWindowType(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

static XCBCookie
GetSizeHints(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMNormalHintsCookie(display, win);
}

static XCBCookie
GetWMHints(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMHintsCookie(display, win);
}

static XCBCookie
GetWMClass(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMClassCookie(display, win);
}

static XCBCookie
GetWMProtocol(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMProtocolsCookie(display, win, wmatom[WMProtocols]);
}

static XCBCookie
GetStrut(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_LENGTH = 4;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
}

static XCBCookie
GetStrutp(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_P_LENGTH = 12;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
}

static XCBCookie
GetNetWMName(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
}

static XCBCookie
GetWMName(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
}

static XCBCookie
GetPid(XCBDisplay *display, XCBWindow win)
{   return XCBGetPidCookie(display, win, netatom[NetWMPid]);
}

static XCBCookie
GetIcon(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ANY);
}

static XCBCookie
GetMotifHints(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t MOTIF_WM_HINT_LENGTH = 5;
    return XCBGetWindowPropertyCookie(display, win, motifatom, NO_BYTE_OFFSET, MOTIF_WM_HINT_LENGTH, False, motifatom);
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
UpdateProperty(XCBDisplay *display, __Property__Cookie__ *cookie)
{
    /* get the cookie */
    const XCBCookie _XCB_COOKIE = __prop_handler[cookie->type].cookie_getter(display, cookie->win);
    /* get the reply and update prop */
    __prop_handler[cookie->type].reply_getter(display, cookie, _XCB_COOKIE);
}

static void
GrabQueuedItemWorker(__Property__Cookie__ *cookie_return)
{
    /* check for relavent items */
    if(!CQueueIsEmpty(&__threads.queue))
    {
        *cookie_return = __threads.queue_data[__threads.queue.front];
        CQueuePop(&__threads.queue);
    }
}

void *
Worker(void *x)
{
    int screen;
    XCBDisplay *display = XCBOpenDisplay(NULL, &screen);
    __Property__Cookie__ cookie = { .win = 0, .type = 0 };
    if(!display)
    {   
        DEBUG0("Failed to open display for worker");
        return NULL;
    }
    while(cookie.type != PropExitThread)
    {
        /* wait for stuff to happen */
        pthread_mutex_lock(&__threads.queue.mutex);
        pthread_cond_wait(&__threads.queue.cond, &__threads.queue.mutex);
        pthread_mutex_unlock(&__threads.queue.mutex);
        /* grab if any item */
        GrabQueuedItemWorker(&cookie);

        if(cookie.win)
        {   
            UpdateProperty(display, &cookie);
            cookie.win = 0;
        }
    }
    XCBCloseDisplay(display);
    return NULL;
}

/*
 * RETURN: pthread_create() return values.
 */
static int
CreateWorker(pthread_t *id_return)
{
    int ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    /* tbh you could do this with only ~100 bytes of stack */
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    /* No need for stack protection since we just get pointers back, and see above */
    pthread_attr_setguardsize(&attr, 0);
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
    pthread_t id = 0;
    for(i = 0; i < threads; ++i)
    {   CreateWorker(&__threads.threads[i]);
    }
}

static void
PropDestroyWorkers(uint32_t threads)
{
    uint32_t i;
    for(i = 0; i < threads; ++i)
    {   
        if(CQueueIsFull(&__threads.queue))
        {   CQueuePop(&__threads.queue);
        }
        PropListen(NULL, 0, PropExitThread);
    }
    volatile uint32_t j;
    for(j = 0; j < threads; ++j)
    {   
        pthread_cancel(__threads.threads[j]);
    }
}

void 
PropInit(void)
{
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
    if(__threads.use_threads)
    {   PropDestroyWorkers(PropGetThreadCount());
    }
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
    /* single thread operation */
    if(!usethreads || full)
    {   
        UpdateProperty(display, &cookie);
        DEBUG("Using single threads, full: %d", full);
    }
}
