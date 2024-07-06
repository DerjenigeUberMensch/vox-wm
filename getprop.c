#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include "XCB-TRL/xcb_winutil.h"
#include "queue.h"
#include "propertynotify.h"
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
        pthread_mutex_lock(&_wm.mutex);
    }
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
            break;
        case PropWindowType:
            break;
        case PropSizeHints:
            break;
        case PropWMHints:
            break;
        case PropWMClass:
            break;
        case PropWMProtocol:
            break;
        case PropStrut:
            break;
        case PropStrutp:
            break;
        case PropNetWMName:
            break;
        case PropWMName:
            break;
        case PropPid:
            break;
        case PropIcon:
            break;
        case PropMotifHints:
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

static void *
Worker(void *x)
{
    int screen;
    XCBDisplay *display = XCBOpenDisplay(NULL, &screen);
    __Property__Cookie__ cookie = { .win = 0, .type = 0 };
    if(!display)
    {   return NULL;
    }
    while(cookie.type != PropExitThread)
    {
        /* wait for stuff to happen */
        pthread_mutex_lock(&__queue.mutex);
        pthread_cond_wait(&__queue.cond, &__queue.mutex);

        /* grab if any item */
        GrabQueuedItemWorker(&cookie);

        pthread_mutex_unlock(&__queue.mutex);
        if(cookie.win)
        {   
            UpdateProperty(display, &cookie);
            cookie.win = 0;
        }
    }
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
    {
        PropListen(NULL, 0, PropExitThread);
        pthread_join(__threads[i], NULL);
    }
}

void 
PropInit(void)
{
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
    PropDestroyWorkers(PropGetThreadCount());
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
