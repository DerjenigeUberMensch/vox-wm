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

struct  __Property__Cookie__
{
    XCBWindow win;
    enum PropertyType type;
};

static __Property__Cookie__ __queue__data[QUEUE_SIZE];
static CQueue __queue; 
static pthread_t __threads[MAX_THREADS];
static int USING_THREADS = 1;


extern WM _wm;
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];
extern XCBAtom motifatom;



static void
LockMainThread()
{   
    if(_wm.use_threads)
    {   pthread_mutex_lock(&_wm.mutex);
    }
}

static void
UnlockMainThread()
{   
    if(_wm.use_threads)
    {   pthread_mutex_unlock(&_wm.mutex);
    }
}


static const XCBCookie
__PROP_GET_COOKIE(XCBDisplay *display, XCBWindow win, enum PropertyType type)
{
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    const uint8_t STRUT_P_LENGTH = 12;
    const uint8_t STRUT_LENGTH = 4;
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t MOTIF_WM_HINT_LENGTH = 5;

    /* These dont require mutex for the following reasons:
     * - They are stack allocated.
     * - They are set during setup() before propertynotify is intialized.
     * - They are never changed afterwards.
     * - These threads are killed before exit, to prevent the stack from de-initializing these variables.
     */
    (void)netatom;
    (void)wmatom;
    (void)motifatom;

    XCBCookie ret = { .sequence = 0 };
    switch(type)
    {
        case PropTransient:
            ret = XCBGetTransientForHintCookie(display, win);
            break;
        case PropWindowState:
            ret = XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
            break;
        case PropWindowType:
            ret = XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
            break;
        case PropSizeHints:
            ret = XCBGetWMNormalHintsCookie(display, win);
            break;
        case PropWMHints:
            ret = XCBGetWMHintsCookie(display, win);
            break;
        case PropWMClass:
            ret = XCBGetWMClassCookie(display, win);
            break;
        case PropWMProtocol:
            ret = XCBGetWMProtocolsCookie(display, win, wmatom[WMProtocols]);
            break;
        case PropStrut:
            ret = XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
            break;
        case PropStrutp:
            ret = XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
            break;
        case PropNetWMName:
            ret = XCBGetWindowPropertyCookie(display, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
            break;
        case PropWMName:
            ret = XCBGetWindowPropertyCookie(display, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
            break;
        case PropPid:
            ret = XCBGetPidCookie(display, win, netatom[NetWMPid]);
            break;
        case PropIcon:
            ret = XCBGetWindowPropertyCookie(display, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ANY);
            break;
        case PropMotifHints:
            ret = XCBGetWindowPropertyCookie(display, win, motifatom, NO_BYTE_OFFSET, MOTIF_WM_HINT_LENGTH, False, motifatom);
            break;

        /* ignore these cases */
        case PropNone:
        case PropLAST:
        default:
            break;
    }
    return ret;
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
            arrange(c->desktop);
            XCBFlush(_wm.dpy);
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
            clientinitwtype(c, prop);
            arrange(c->desktop);
            XCBFlush(_wm.dpy);
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
            clientinitwstate(c, prop);
            arrange(c->desktop);
            XCBFlush(_wm.dpy);
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
        {   
            updatesizehints(c, &hints);
            arrange(c->desktop);
            XCBFlush(_wm.dpy);
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
        {   
            updatewmhints(c, prop);
            arrange(c->desktop);
            XCBFlush(_wm.dpy);
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
}

static void
UpdateStrutP(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
    /* TODO */
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
        {   
            updatemotifhints(c, prop);
            arrange(c->desktop);
            XCBFlush(_wm.dpy);
        }
        UnlockMainThread();
    }
    free(prop);
}

static void
__X__(XCBDisplay *display, __Property__Cookie__ *cookie, XCBCookie _XCB_COOKIE)
{
}

static void
UpdateProperty(XCBDisplay *display, __Property__Cookie__ *cookie)
{
    const XCBCookie _XCB_COOKIE = __PROP_GET_COOKIE(display, cookie->win, cookie->type);
    switch(cookie->type)
    {
        case PropTransient:
            UpdateTrans(display, cookie, _XCB_COOKIE);
            break;
        case PropWindowState:
            UpdateWindowState(display, cookie, _XCB_COOKIE);
            break;
        case PropWindowType:
            UpdateWindowType(display, cookie, _XCB_COOKIE);
            break;
        case PropSizeHints:
            UpdateSizeHints(display, cookie, _XCB_COOKIE);
            break;
        case PropWMHints:
            UpdateWMHints(display, cookie, _XCB_COOKIE);
            break;
        case PropWMClass:
            UpdateWMClass(display, cookie, _XCB_COOKIE);
            break;
        case PropWMProtocol:
            UpdateWMProtocol(display, cookie, _XCB_COOKIE);
            break;
        case PropStrut:
            UpdateStrut(display, cookie, _XCB_COOKIE);
            break;
        case PropStrutp:
            UpdateStrutP(display, cookie, _XCB_COOKIE);
            break;
        case PropNetWMName:
            UpdateNetWMName(display, cookie, _XCB_COOKIE);
            break;
        case PropWMName:
            UpdateWMName(display, cookie, _XCB_COOKIE);
            break;
        case PropPid:
            UpdatePid(display, cookie, _XCB_COOKIE);
            break;
        case PropIcon:
            UpdateIcon(display, cookie, _XCB_COOKIE);
            break;
        case PropMotifHints:
            UpdateMotifHints(display, cookie, _XCB_COOKIE);
            break;
        /* ignore these cases */
        case PropNone:
        case PropLAST:
        default:
            break;
    }
}

static void
GrabQueuedItemWorker(__Property__Cookie__ *cookie_return)
{
    /* check for relavent items */
    if(!CQueueIsEmpty(&__queue))
    {
        *cookie_return = __queue__data[__queue.front];
        CQueuePop(&__queue);
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
        pthread_mutex_lock(&__queue.mutex);
        pthread_cond_wait(&__queue.cond, &__queue.mutex);
        pthread_mutex_unlock(&__queue.mutex);
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
    {   aloc_threads = cores;
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
    {   CreateWorker(&__threads[i]);
    }
}

static void
PropDestroyWorkers(uint32_t threads)
{
    uint32_t i;
    for(i = 0; i < threads; ++i)
    {   PropListen(NULL, 0, PropExitThread);
    }
    volatile uint32_t j;
    for(j = 0; j < threads; ++j)
    {   pthread_join(__threads[j], NULL);
    }
}

void 
PropInit(void)
{
    if(!_wm.use_threads)
    {   
        USING_THREADS = 0;
        return;
    }
    uint8_t ret = CQueueCreate((void **)&__queue__data, QUEUE_SIZE, sizeof(__Property__Cookie__), &__queue);
    if(ret)
    {   USING_THREADS = 0;
    }
    else
    {   PropCreateWorkers(PropGetThreadCount());
    }
}

void
PropDestroy(void)
{
    if(USING_THREADS)
    {   PropDestroyWorkers(PropGetThreadCount());
    }
}

void 
PropListen(XCBDisplay *display, XCBWindow win, enum PropertyType type)
{
    __Property__Cookie__ cookie;
    cookie.win = win;
    cookie.type = type;
    if(USING_THREADS && _wm.use_threads)
    {   CQueueAdd(&__queue, (void *)&cookie);
    }
    else /* single thread operation */
    {   UpdateProperty(display, &cookie);
    }
}
