#include "pannel.h"
#include "XCB-TRL/xcb_image.h"
#include "XCB-TRL/xcb_imgutil.h"
#include "dwm.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


static int 
PannelLock(Pannel *p)
{
    return pthread_mutex_lock(p->mut);
}

static int
PannelUnlock(Pannel *p)
{
    return pthread_mutex_unlock(p->mut);
}

static int 
PannelLockQ(Pannel *p)
{
    return pthread_spin_lock(p->qmut);
}

static int
PannelUnlockQ(Pannel *p)
{
    return pthread_spin_unlock(p->qmut);
}

static int 
PannelCondWait(Pannel *p)
{   
    PannelLock(p);
    return pthread_cond_wait(p->cond, p->mut);
}

static int
PannelCondUnwait(Pannel *p)
{
    pthread_cond_signal(p->cond);
    return PannelUnlock(p);
}

static void
PannelGraphicsExpose(Pannel *p, XCBGenericEvent *_x)
{
    XCBGraphicsExposeEvent *ev  = (XCBGraphicsExposeEvent *)_x;
    const i16 x                 = ev->x;
    const i16 y                 = ev->y;
    const u16 w                 = ev->width;
    const u16 h                 = ev->height;
    const u16 count             = ev->count;
    const XCBDrawable drawable  = ev->drawable;
    const u8 majoropcode        = ev->major_opcode;
    const u16 minoropcode       = ev->minor_opcode;

    (void)x;
    (void)y;
    (void)count;
    (void)drawable;
    (void)majoropcode;
    (void)minoropcode;

    //PannelDrawBuff(p, x, y, w, h);
    //XCBFlush(p->dpy);
}

static void
PannelExpose(Pannel *p, XCBGenericEvent *_x)
{
    XCBExposeEvent *ev      = (XCBExposeEvent *)_x;
    const XCBWindow win     = ev->window;
    const i16 x             = ev->x;
    const i16 y             = ev->y;
    const u16 w             = ev->width;
    const u16 h             = ev->height;
    const u16 count         = ev->count;

    if(!count && p->win == win)
    {   
        DEBUG("(x: %d, y: %d) -> (w: %u, h: %u)", x, y, w, h);
        PannelDrawBuff(p, x, y, w, h);
        XCBFlush(p->dpy);
    }
}



static void
PannelResizeRequest(Pannel *p, XCBGenericEvent *_x)
{
    XCBResizeRequestEvent *ev = (XCBResizeRequestEvent *)_x;
    const XCBWindow win = ev->window;
    const u16 w         = ev->width;
    const u16 h         = ev->height;

    if(win == p->win && (w != p->w || h != p->h))
    {
        PannelResizeBuff(p, w, h);
        XCBFlush(p->dpy);
    }
}



static void *
PannelInitLoop(
        void *pannel
        )
{
    Pannel *p = (Pannel *)pannel;
    XCBGenericEvent *ev = NULL;
    const u32 mask = XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_RESIZE_REDIRECT|XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_FOCUS_CHANGE;
    XCBSelectInput(p->dpy, p->win, mask);
    XCBSync(p->dpy);
    while(p->running && !XCBNextEvent(p->dpy, &ev))
    {
        PannelLock(p);
        switch(XCB_EVENT_RESPONSE_TYPE(ev))
        {
            case XCB_EXPOSE:
                PannelExpose(p, ev);
                break;
            case XCB_GRAPHICS_EXPOSURE:
                PannelGraphicsExpose(p, ev);
                break;
            case XCB_RESIZE_REQUEST:
                PannelResizeRequest(p, ev);
                break;
            case XCB_NONE:
                xerror(p->dpy, (XCBGenericError *)ev);
                break;
        }
        DEBUG("%s", XCBGetEventNameFromEvent(ev));
        free(ev);
        PannelUnlock(p);
    }
    return NULL;
}

/*
 * RETURN: 1 on Failure.
 * RETURN: 0 on Success.
 */
static int
PannelInitThreads(Pannel *p)
{
    p->mut = malloc(sizeof(pthread_mutex_t));
    p->qmut = malloc(sizeof(pthread_spinlock_t));
    p->cond = malloc(sizeof(pthread_cond_t));

    if(!p->mut || !p->qmut || !p->cond)
    {   goto FAILURE;
    }
    pthread_mutexattr_t recursive_attr;
    pthread_mutexattr_init(&recursive_attr);
    pthread_mutexattr_settype(&recursive_attr, PTHREAD_MUTEX_RECURSIVE);
    if(pthread_mutex_init(p->mut, &recursive_attr))
    {   goto FAILURE;
    }
    else if(pthread_spin_init(p->qmut, 1))
    {   
        pthread_mutex_destroy(p->mut);
        goto FAILURE;
    }
    else if(pthread_cond_init(p->cond, NULL))
    {   
        pthread_mutex_destroy(p->mut);
        pthread_spin_destroy(p->qmut);
        goto FAILURE;
    }
    return 0;
FAILURE:
    free(p->mut);
    free((void *)p->qmut);
    free(p->cond);
    return 1;
}

static void
PannelDestroyThreads(Pannel *pannel)
{
    pthread_mutex_destroy(pannel->mut);
    pthread_spin_destroy(pannel->qmut);
    pthread_cond_destroy(pannel->cond);
    free(pannel->mut);
    free((void *)pannel->qmut);
    free(pannel->cond);
}

void
PannelRedraw(Pannel *pannel)
{
    PannelLock(pannel);
    /* TODO */
    PannelDrawBuff(pannel, 0, 0, pannel->w, pannel->h);
    /* stuff */
    PannelUnlock(pannel);
}

int
PannelResizeBuff(
        Pannel *pannel,
        uint32_t w,
        uint32_t h
        )
{
    PannelLock(pannel);
    if(w > pannel->pixw || h > pannel->pixh)
    {
        XCBFreePixmap(pannel->dpy, pannel->pix);
        pannel->pix = XCBCreatePixmap(pannel->dpy, XCBRootWindow(pannel->dpy, pannel->screen), w, h, XCBDefaultDepth(pannel->dpy, pannel->screen));
        if(pannel->pix)
        {   
            pannel->pixw = w;
            pannel->pixh = h;
        }
    }
    if(pannel->pix)
    {   
        pannel->w = w;
        pannel->h = h;
    }
           
    pannel->w = w;
    pannel->h = h;
    PannelUnlock(pannel);
    return !pannel->pix;
}


static XCBWindow
PannelCreateWindow(
        XCBDisplay *display, 
        int screen,
        XCBWindow parent, 
        int32_t x, 
        int32_t y, 
        int32_t w, 
        int32_t h
        )
{
    const uint8_t red = 0;
    const uint8_t green = 0;
    const uint8_t blue = 0;
    const uint8_t alpha = 0;    /* transparency, 0 is opaque, 255 is fully transparent */
    const uint32_t bordercolor = blue + (green << 8) + (red << 16) + (alpha << 24);
    const uint32_t backgroundcolor = bordercolor;
    const uint32_t borderwidth = 0;
    const uint8_t depth = XCBDefaultDepth(display, screen);
    const uint32_t border_width = 0;
    const XCBVisual visual = XCBGetScreen(display)->root_visual;
    const uint32_t winmask = XCB_CW_BACKING_STORE|XCB_CW_BACK_PIXEL|XCB_CW_BORDER_PIXEL;

    XCBCreateWindowValueList winval = 
    {   
        .background_pixel = backgroundcolor,
        .border_pixel = bordercolor,
        .backing_store = XCB_BACKING_STORE_WHEN_MAPPED,
    };
    XCBWindow ret = XCBCreateWindow(display, parent, x, y, w, h, border_width, depth, XCB_WINDOW_CLASS_INPUT_OUTPUT, visual, winmask, &winval);

    XCBSizeHints hints = 
    {
        .min_width = 1,
        .min_height = 1,
        .flags = XCB_SIZE_HINT_P_MIN_SIZE
    };
    XCBClassHint classhint =
    {
        .instance_name = "Dev-Pannel",
        .class_name = "Pannel"
    };

    XCBSetWMNormalHints(display, ret, &hints);
    XCBSetClassHint(display, ret, &classhint);
    return ret;
}

static XCBGC
PannelCreateGC(
        XCBDisplay *display,
        int screen
        )
{
    XCBCreateGCValueList gcval = { .foreground = XCBBlackPixel(display, screen) };
    
    XCBGC gc;

    gc = XCBCreateGC(display, XCBRootWindow(display, screen), XCB_GC_FOREGROUND, &gcval);

    return gc;
}

/* Builtin pannel Stuff */
Pannel *
PannelCreate(
    XCBWindow parent, 
    int32_t x, 
    int32_t y, 
    int32_t w, 
    int32_t h
    )
{
    int screen = 0; 
    XCBDisplay *display = XCBOpenDisplay(NULL, &screen);
    if(!display)
    {   return NULL;
    }
    Pannel *pannel = malloc(sizeof(Pannel));
    if(pannel)
    {
        pannel->running = 1;
        pannel->x = x;
        pannel->y = y;
        pannel->w = w;
        pannel->h = h;
        pannel->win = PannelCreateWindow(display, screen, parent, x, y, w, h);
        pannel->gc = PannelCreateGC(display, screen);
        pannel->pix = XCBCreatePixmap(display, XCBRootWindow(display, screen), w, h, XCBDefaultDepth(display, screen));
        pannel->dpy = display;
        pannel->screen = screen;
        pannel->pixw = w;
        pannel->pixh = h;
        pannel->widgets = NULL;
        pannel->widgetslen = 0;
        XCBSetLineAttributes(display, pannel->gc, 1, XCB_LINE_STYLE_SOLID, XCB_CAP_STYLE_BUTT, XCB_JOIN_STYLE_MITER);
        if(PannelInitThreads(pannel))
        {   
            XCBDestroyWindow(pannel->dpy, pannel->win);
            XCBFreePixmap(pannel->dpy, pannel->pix);
            XCBFreeGC(pannel->dpy, pannel->gc);
            free(pannel);
            pannel = NULL;
        }
        else
        {   
            pthread_t id;
            if(pthread_create(&id, NULL, PannelInitLoop, pannel))
            {   PannelDestroy(pannel);
                pannel = NULL;
            }
            else
            {   pthread_detach(id);
            }
        }
    }
    return pannel;
}

void
PannelDestroy(
    Pannel *pannel
        )
{
    uint32_t i;
    PannelLock(pannel);
    pannel->running = 0;
    wakeupconnection(pannel->dpy, pannel->screen);
    PannelUnlock(pannel);

    /* hopefully this works */
    PannelLock(pannel);
    XCBDestroyWindow(pannel->dpy, pannel->win);
    XCBFreeGC(pannel->dpy, pannel->gc);
    XCBFreePixmap(pannel->dpy, pannel->pix);
    XCBFlush(pannel->dpy);
    PannelUnlock(pannel);
    for(i = 0 ; i < pannel->widgetslen; ++i)
    {   XCBFreePixmap(pannel->dpy, (((PannelWidget *)pannel->widgets) + i)->pix);
    }
    free(pannel->widgets);
    PannelDestroyThreads(pannel);
    XCBCloseDisplay(pannel->dpy);
    free(pannel);   
}

PannelWidget *
PannelWidgetCreate(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        uint16_t w,
        uint16_t h
        )
{
    PannelLock(pannel);
    size_t offset = pannel->widgetslen * sizeof(PannelWidget);
    /* + sizeof(PannelWidget) cause were creating a new one */
    size_t newsize = offset + sizeof(PannelWidget);
    uint8_t *realoc = realloc(pannel->widgets, newsize);
    PannelWidget *widget = NULL;
    if(realoc)
    {           /* cast cause compiler throws warning */
        widget = (PannelWidget *)((uint8_t *)realoc + offset);
        widget->x = x;
        widget->y = y;
        widget->w = w;
        widget->h = h;
        widget->pix = XCBCreatePixmap(pannel->dpy, XCBRootWindow(pannel->dpy, pannel->screen), w, h, XCBDefaultDepth(pannel->dpy, pannel->screen));
        widget->pixw = w;
        widget->pixh = h;
        if(!widget->pix)
        {   widget = NULL;
        }
    }
    PannelUnlock(pannel);
    return widget;
}

/* Resizes a widget based on parametors. Widget is assumed to be empty, as data WILL be destroyed.
 * 
 * RETURN: 1 on Failure.
 * RETURN: 0 on Success.
 */
int
PannelWidgetResize(
        Pannel *pannel,
        PannelWidget *widget,
        uint16_t w,
        uint16_t h
        )
{
    PannelLock(pannel);
    if(w > widget->pixw || h > widget->pixh)
    {   
        XCBFreePixmap(pannel->dpy, widget->pix);
        widget->pix = XCBCreatePixmap(pannel->dpy, XCBRootWindow(pannel->dpy, pannel->screen), w, h, XCBDefaultDepth(pannel->dpy, pannel->screen));
        if(widget->pix)
        {   
            widget->pixw = w;
            widget->pixh = h;
        }
    }
    if(widget->pix)
    {   
        widget->w = w;
        widget->h = h;
    }
    PannelUnlock(pannel);
    return !widget->pix;
}

void
PannelWritePixel(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        uint32_t colour
        )
{
    PannelLock(pannel);
    XCBPutPixels(pannel->dpy, pannel->gc, pannel->pix, XCB_IMAGE_FORMAT_BGRA, 32, 0, x, y, 1, 1, 4, (const uint8_t *)&colour);
}

void
PannelDrawLine(
        Pannel *pannel,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        uint32_t colour
        )
{
    PannelLock(pannel);
    XCBSetForeground(pannel->dpy, pannel->gc, colour);
    XCBDrawLine(pannel->dpy, XCB_COORD_MODE_ORIGIN, pannel->pix, pannel->gc, startx, starty, endx, endy);
    /* TODO */
    PannelUnlock(pannel);
}

void
PannelDrawRectangle(
        Pannel *pannel,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        uint8_t fill,
        uint32_t colour
        )
{
    PannelLock(pannel);
    XCBSetForeground(pannel->dpy, pannel->gc, colour);
    if(fill)
    {   XCBFillRectangle(pannel->dpy, pannel->pix, pannel->gc, x, y, w, h);
    }
    else
    {   XCBDrawRectangle(pannel->dpy, pannel->pix, pannel->gc, x, y, w, h);
    }
    PannelUnlock(pannel);
}

void
PannelWidgetDrawLine(
        Pannel *pannel,
        PannelWidget *widget,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        uint32_t colour
        )
{
    PannelLock(pannel);
    XCBSetForeground(pannel->dpy, pannel->gc, colour);
    XCBDrawLine(pannel->dpy, XCB_COORD_MODE_ORIGIN, widget->pix, pannel->gc, startx, starty, endx, endy);
    PannelUnlock(pannel);
}

void
PannelWidgetDrawRectangle(
        Pannel *pannel,
        PannelWidget *wi,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        uint8_t fill,
        uint32_t colour
        )
{
    PannelLock(pannel);
    XCBSetForeground(pannel->dpy, pannel->gc, colour);

    if(fill)
    {   XCBFillRectangle(pannel->dpy, wi->pix, pannel->gc, x, y, w, h);
    }
    else
    {   XCBDrawRectangle(pannel->dpy, wi->pix, pannel->gc, x, y, w, h);
    }

    PannelUnlock(pannel);
}

void
PannelSmoothBuff(
        Pannel *pannel,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        )
{

}

/* TOD */
XCBCookie
PannelDrawBuff(
    Pannel *pannel,
    int16_t xoffset,
    int16_t yoffset,
    const uint16_t w,
    const uint16_t h
        )
{
    PannelLock(pannel);
    const i32 x = xoffset;
    const i32 y = yoffset;
    XCBCookie ret = XCBCopyArea(pannel->dpy, pannel->pix, pannel->win, pannel->gc, x, y, w, h, 0, 0);
    PannelUnlock(pannel);
    return ret;
}



/*
 * id1 id2 id3 * 256
 * I think the only real solution is to have the client print from back to frot.
 * EX:
 * last lastfocus laststack
 * And just print down from there disregardning the client, but the client just has info of the client .
 * Some issues may arrise if the window is 0, this also is implementation reliant.
 * TODO
 */
