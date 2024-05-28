#include "pannel.h"
#include "XCB-TRL/xcb_image.h"
#include "XCB-TRL/xcb_imgutil.h"
#include "dwm.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


static void
VALUE_CLAMP_PANNEL(Pannel *p, i32 *x, i32 *y, u32 *w, u32 *h)
{
    /* make sure coords are in possible space */
    *x = MAX(*x, p->x);
    *x = MIN(*x, p->x + p->w);
    *y = MAX(*y, p->y);
    *y = MIN(*y, p->y + p->h);

    /* handle too big values */
    if(*w + *x > p->w)
    {   *w -= *x;
    }
    if(*h + *y > p->h)
    {   *h -= *y;
    }
}

static void
VALUE_CLAMP_WIDGET(PannelWidget *wi, i32 *x, i32 *y, u32 *w, u32 *h)
{
    /* make sure coords are in possible space */
    *x = MAX(*x, wi->x);
    *x = MIN(*x, wi->x + wi->w);
    *y = MAX(*y, wi->y);
    *y = MIN(*y, wi->y + wi->h);

    /* handle too big values */
    if(*w + *x > wi->w)
    {   *w -= *x;
    }
    if(*h + *y > wi->h)
    {   *h -= *y;
    }
}

static void
PLOT_PANNEL(Pannel *p, i32 x, i32 y, void *data)
{
    /* bounds check */
    if(y * p->w + x < p->w * p->h)
    {   memcpy(p->buff + (y * p->w + x) * p->itemsize, data, p->itemsize);
    }
    else
    {   DEBUG("Failed to plot point. (%d, %d)", x, y);
    }
}

static void
PLOT_PANNEL_POINTS(Pannel *p, i32 x, i32 y, u32 w, u32 h, void *data)
{
    u64 _x, _y;
    for(_x = 0; _x < w; ++_x)
    {
        for(_y = 0; _y < h; ++_y)
        {   
            PLOT_PANNEL(p, x + _x, y + _y, ((uint8_t *)data) + ((_y * w + _x) * p->itemsize));
        }
    }
}

static void
PLOT_WIDGET(PannelWidget *wi, i32 x, i32 y, void *data)
{
    /* bounds check */
    if(x * wi->h + x < wi->w * wi->h)
    {   memcpy(wi->buff + (y * wi->w + x) * wi->itemsize, data, wi->itemsize);
    }
}

static void
PLOT_WIDGET_POINTS(PannelWidget *wi, i32 x, i32 y, u32 w, u32 h, void *data)
{
    u64 _x, _y;
    for(_x = 0; _x < w; ++_x)
    {
        for(_y = 0; _y < h; ++_y)
        {   
            PLOT_WIDGET(wi, x + _x, y + _y, ((uint8_t *)data) + ((_y * w + _x) * wi->itemsize));
        }
    }
}

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
    PannelDrawBuff(p, x, y, w, h);
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
    PannelDrawBuff(p, x, y, w, h);
}

static void
PannelConfigureNotify(Pannel *p, XCBGenericEvent *_x)
{
    XCBConfigureNotifyEvent *ev = (XCBConfigureNotifyEvent *)_x;
    const XCBWindow eventwin = ev->event;
    const XCBWindow win = ev->window;
    const XCBWindow abovesibling = ev->above_sibling;
    const i16 x = ev->x;
    const i16 y = ev->y;
    const u16 w = ev->width;
    const u16 h = ev->height;
    const u16 borderwidth = ev->border_width;
    const u8 overrideredirect = ev->override_redirect;

    if(p->win == win)
    {
        p->x = x;
        p->y = y;
        p->w = w;
        p->h = h;
        PannelRedraw(p);
    }
}


static void *
PannelInitLoop(
        void *pannel
        )
{
    Pannel *p = (Pannel *)pannel;
    XCBGenericEvent *ev = NULL;
    XCBSelectInput(p->dpy, p->win, XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_FOCUS_CHANGE|XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|XCB_EVENT_MASK_STRUCTURE_NOTIFY);
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
            case XCB_CONFIGURE_NOTIFY:
                PannelConfigureNotify(p, ev);
                break;
            case XCB_NONE:
                xerror(p->dpy, (XCBGenericError *)ev);
                break;

        }
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

    if(!p->mut || p->qmut || !p->cond)
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
    /* stuff */
    PannelUnlock(pannel);
}



/*
 * NOTE: No side effects if resize fails.
 *
 * RETURN: 0 On Success.
 * RETURN: 1 On Failure.
 */
int
PannelResizeBuff(
        Pannel *pannel,
        uint32_t w,
        uint32_t h
        )
{
    PannelLock(pannel);
    if(w * h > pannel->bufflength)
    {
        const u64 length = w * h;
        void *realoc = realloc(pannel->buff, length * pannel->itemsize);
        if(realoc)
        {   pannel->buff = realoc;
            pannel->bufflength = length;
        }
        else    /* dont change anything exit failure */
        {   return 1;
        }
    }
    pannel->w = w;
    pannel->h = h;
    PannelUnlock(pannel);
    return 0;
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
    const uint8_t red = 0;
    const uint8_t green = 0;
    const uint8_t blue = 0;
    const uint8_t alpha = 0;    /* transparency, 0 is opaque, 255 is fully transparent */
    const uint32_t bordercolor = blue + (green << 8) + (red << 16) + (alpha << 24);
    const uint32_t backgroundcolor = bordercolor;
    const uint32_t borderwidth = 0;
    XCBCreateGCValueList gcval = { .foreground = XCBBlackPixel(display, screen) };

    if(pannel)
    {
        pannel->x = x;
        pannel->y = y;
        pannel->w = w;
        pannel->h = h;
        pannel->win = XCBCreateSimpleWindow(display, parent, x, y, w, h, borderwidth, bordercolor, backgroundcolor);
        pannel->gc = XCBCreateGC(display, XCBRootWindow(display, screen), XCB_GC_FOREGROUND, &gcval);
        pannel->dpy = display;
        pannel->screen = screen;
        pannel->itemsize = sizeof(uint32_t);
        pannel->bufflength = w * h;
        pannel->buff = calloc(1, pannel->bufflength * pannel->itemsize);
        pannel->widgets = NULL;
        pannel->widgetslen = 0;
        XCBSetLineAttributes(display, pannel->gc, 1, XCB_LINE_STYLE_SOLID, XCB_CAP_STYLE_BUTT, XCB_JOIN_STYLE_MITER);
        if(!pannel->buff || PannelInitThreads(pannel))
        {   
            XCBDestroyWindow(pannel->dpy, pannel->win);
            XCBFreeGC(pannel->dpy, pannel->gc);
            free(pannel->buff);
            free(pannel);
            pannel = NULL;
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
    XCBFlush(pannel->dpy);
    XCBCloseDisplay(pannel->dpy);
    PannelUnlock(pannel);
    /* cleanup */
    free(pannel->buff);
    for(i = 0 ; i < pannel->widgetslen; ++i)
    {   free((((PannelWidget *)pannel->widgets) + i)->buff);
    }
    free(pannel->widgets);
    PannelDestroyThreads(pannel);
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
        widget->itemsize = pannel->itemsize;
        widget->bufflength = w * h;
        widget->buff = calloc(1, widget->bufflength * widget->itemsize);
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
    u8 ret = 1;
    if(w * h > widget->bufflength)
    {
        const u64 length = w * h;
        void *realoc = realloc(widget->buff, length * widget->itemsize);
        if(realoc)
        {   
            widget->buff = realoc;
            widget->bufflength = length;
            widget->w = w;
            widget->h = h;
            ret = 0;
        }
    }
    else
    {   
        widget->w = w;
        widget->h = h;
        ret = 0;
    }
    PannelUnlock(pannel);
    return ret;
}

void
PannelWidgetWrite(
        Pannel *pannel,
        PannelWidget *widget,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        void *data
        )
{
    PannelLock(pannel);
    VALUE_CLAMP_WIDGET(widget, &x, &y, &w, &h);
    uint32_t coordoffset = (y * widget->w) + x;
    uint32_t itemsize = widget->itemsize;
    size_t byteoffset = coordoffset * itemsize;
    size_t size = itemsize * w * h;

    /* make sure its not empty/negative */
    if(size > 0)
    {   memcpy(((uint8_t *)widget->buff) + (byteoffset), data, size);
    }
    PannelUnlock(pannel);
}

int
PannelWritePixel(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        void *colour
        )
{
    PannelLock(pannel);
    u8 check = x * y < pannel->w * pannel->h;
    if(check)
    {   PLOT_PANNEL(pannel, x, y, colour);
    }
    PannelUnlock(pannel);
    return !check;
}

int
PannelWriteBuff(
        Pannel *pannel,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        void *data
        )
{
    PannelLock(pannel);
    VALUE_CLAMP_PANNEL(pannel, &x, &y, &w, &h);

    size_t size = pannel->itemsize * w * h;
    /* make sure its not empty/negative */
    if(size > 0)
    {   PLOT_PANNEL_POINTS(pannel, x, y, w, h, data);
    }
    PannelUnlock(pannel);
    return size < 0;
}


void
PannelDrawLine(
        Pannel *pannel,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        void *colour 
        )
{
    PannelLock(pannel);
    i32 dx = abs(endx - startx);
    i32 dy = abs(endy - starty);
    i32 sx = startx < endx ? 1 : -1;
    i32 sy = starty < endy ? 1 : -1;
    i32 err = (dx > dy ? dx : -dy) / 2;
    i32 e2;

    while (1) {
        PLOT_PANNEL(pannel, startx, starty, colour); // Plot the current pixel
        if (startx == endx && starty == endy) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            startx += sx;
        }
        if (e2 < dy) {
            err += dx;
            starty += sy;
        }
    }
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
        void *colour
        )
{
    if(!w || !h)
    {   return;
    }
    PannelLock(pannel);
    if(fill)
    {
        int64_t _y;
        for(_y = 0; _y < h; ++_y)
        {   PannelDrawLine(pannel, x, _y, x + w, _y, colour);
        }
    }
    /* top */
    PannelDrawLine(pannel, x, y, x + w, y, colour);
    /* bottom */
    PannelDrawLine(pannel, x, y + h, x + w, y + h, colour);
    /* left side */
    PannelDrawLine(pannel, x, y, x, y + h, colour);
    /* right side */
    PannelDrawLine(pannel, x + w, y, x + w, y + h, colour);

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
        void *colour
        )
{
    PannelLock(pannel);
    int dx = abs(endx - startx);
    int dy = abs(endy - starty);
    int sx = startx < endx ? 1 : -1;
    int sy = starty < endy ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while (1) {
        PLOT_WIDGET(widget, startx, starty, colour); // Plot the current pixel
        if (startx == endx && starty == endy) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            startx += sx;
        }
        if (e2 < dy) {
            err += dx;
            starty += sy;
        }
    }
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
        void *colour 
        )
{
    if(!w || !h)
    {   return;
    }

    PannelLock(pannel);
    if(fill)
    {
        int64_t _y;
        for(_y = 0; _y < h; ++_y)
        {   PannelWidgetDrawLine(pannel, wi, x, _y + y, x + w, _y + y, colour);
        }
    }
    /* top */
    PannelWidgetDrawLine(pannel, wi, x, y, x + w, y, colour);
    /* bottom */
    PannelWidgetDrawLine(pannel, wi, x, y + h, x + w, y + h, colour);
    /* left side */
    PannelWidgetDrawLine(pannel, wi, x, y, x, y + h, colour);
    /* right side */
    PannelWidgetDrawLine(pannel, wi, x + w, y, x + w, y + h, colour);
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
    const unsigned int screen = pannel->screen;
    XCBDisplay *display = pannel->dpy;
    const XCBGC gc = pannel->gc;
    const XCBWindow win = pannel->win;
    const u8 format = XCB_IMAGE_FORMAT_BGRA;
    const u8 depth = XCBDefaultDepth(display, screen);
    const u8 left_pad = 0;
    const i32 x = xoffset;
    const i32 y = yoffset;
    const u32 size = w * h * sizeof(uint32_t);
    const uint8_t *data = (uint8_t *)pannel->buff;
    u32 i;
    for(i = 0; i < pannel->widgetslen; ++i)
    {   
        PannelWidget *widget = pannel->widgets + i;
        PLOT_PANNEL_POINTS(pannel, widget->x, widget->y, widget->w, widget->h, widget->buff);
    }
    XCBCookie ret = XCBPutPixels(display, gc, win, format, depth, left_pad, x, y, w, h, size, data);
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
