#include "pannel.h"
#include "XCB-TRL/xcb_image.h"
#include "XCB-TRL/xcb_imgutil.h"
#include "dwm.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "fonts.h"

extern XCBAtom netatom[NetLast];

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

    static i16 prevx = 0;
    static i16 prevy = 0;
    static u16 prevw = 0;
    static u16 prevh = 0;

    static u8 set = 0;

    static long long int deltatime = 0;     /* miliseconds */


    const u16 FPS = 60;
    const u16 TIME_COVERSION_RATIO = 1000;
    const u8 REDRAW_MIN_TIME_PASSED_MILLI_SECONDS = (TIME_COVERSION_RATIO / FPS);
    const u8 REDRAW_MIN_COUNT = 1;

    int curtime = 0;

    if(p->win == win)
    {
        struct timeval _tim;
        if(!set)
        {   
            prevx = x;
            prevy = y;
            prevw = w;
            prevh = h;
            /* reset deltatime */
            gettimeofday(&_tim, NULL);
            deltatime = _tim.tv_usec * TIME_COVERSION_RATIO;
        }
        else
        {   
            prevx = MIN(prevx, x);
            prevy = MIN(prevy, y);
            prevw = MAX(prevw, w);
            prevh = MAX(prevh, h);
        }

        gettimeofday(&_tim, NULL);
        curtime = _tim.tv_usec * TIME_COVERSION_RATIO;

        int redraw = curtime - deltatime > REDRAW_MIN_TIME_PASSED_MILLI_SECONDS;

        if(redraw || count <= REDRAW_MIN_COUNT)
        {
            DEBUG("(x: %d, y: %d) -> (w: %u, h: %u)", x, y, w, h);
            PannelRedraw(p, prevx, prevy, prevw, prevh);
            XCBFlush(p->dpy);
            /* we drew stuff reset */
            set = 0;
        }
    }
}

static void
PannelUnmapNotify(Pannel *p, XCBGenericEvent *_x)
{
    XCBUnmapNotifyEvent *ev = (XCBUnmapNotifyEvent *)_x;

    if(ev->window == p->win)
    {   p->running = 0;
    }
}

static void
PannelDestroyNotify(Pannel *p, XCBGenericEvent *_x)
{
    XCBDestroyNotifyEvent *ev = (XCBDestroyNotifyEvent *)_x;
    
    if(ev->window == p->win)
    {   p->running = 0;
    }
}

static void
PannelConfigureNotify(Pannel *p, XCBGenericEvent *_x)
{
    XCBConfigureNotifyEvent *ev = (XCBConfigureNotifyEvent *)_x;

    if(ev->window == p->win)
    {
        PannelResizeBuff(p, ev->width, ev->height);
        XCBFlush(p->dpy);
    }
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
    const uint32_t winmask = XCB_CW_BACK_PIXEL|XCB_CW_BORDER_PIXEL;

    XCBCreateWindowValueList winval = 
    {   
        .background_pixel = backgroundcolor,
        .border_pixel = bordercolor,
    };
    XCBWindow ret = XCBCreateWindow(display, parent, x, y, w, h, border_width, depth, XCB_WINDOW_CLASS_INPUT_OUTPUT, visual, winmask, &winval);

    XCBClassHint classhint =
    {
        .instance_name = "Dev-Pannel",
        .class_name = "Pannel"
    };

    XCBSetClassHint(display, ret, &classhint);
    XCBChangeProperty(display, ret, netatom[NetWMWindowType], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_APPEND, (const char *)&netatom[NetWMWindowTypeDialog], 1);
    XCBChangeProperty(display, ret, netatom[NetWMState], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_APPEND, (const char *)&netatom[NetWMStateModal], 1);
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

static void *
PannelInitLoop(
        void *pannel
        )
{
    Pannel *p = (Pannel *)pannel;
    XCBGenericEvent *ev = NULL;
    const u32 mask = 
        XCB_EVENT_MASK_EXPOSURE
        |XCB_EVENT_MASK_BUTTON_PRESS
        |XCB_EVENT_MASK_KEY_PRESS
        |XCB_EVENT_MASK_FOCUS_CHANGE
        |XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        |XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    XCBMapWindow(p->dpy, p->win);
    XCBSelectInput(p->dpy, p->win, mask);
    PannelRedraw(p, 0, 0, p->w, p->h);
    XCBFlush(p->dpy);
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
            case XCB_DESTROY_NOTIFY:
                PannelDestroyNotify(p, ev);
                break;
            case XCB_UNMAP_NOTIFY:
                PannelUnmapNotify(p, ev);
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
    DEBUG0("Pannel Killed");
    PannelDestroy(p);
    return NULL;
}

static void *
PannelInit(void *p)
{
    int screen;
    XCBDisplay *display = XCBOpenDisplay(NULL, &screen);
    if(!display)
    {   return NULL;
    }
    Pannel *pannel = (Pannel *)p;
    uint32_t x = (XCBDisplayWidth(display, screen) / 2) - (pannel->w / 2);
    uint32_t y = (XCBDisplayHeight(display, screen) / 2) - (pannel->h / 2);
    uint32_t sw = XCBDisplayWidth(display, screen);
    uint32_t sh = XCBDisplayHeight(display, screen);
    char *const fontname = "fixed";
    if(pannel)
    {
        pannel->running = 1;
        pannel->win = PannelCreateWindow(display, screen, pannel->win, x, y, pannel->w, pannel->h);
        pannel->gc = PannelCreateGC(display, screen);
        pannel->pix = XCBCreatePixmap(display, XCBRootWindow(display, screen), sw, sh, XCBDefaultDepth(display, screen));
        pannel->dpy = display;
        pannel->screen = screen;
        pannel->pixw = sw;
        pannel->pixh = sh;
        pannel->widgets = NULL;
        pannel->widgetslen = 0;
        pannel->font = XFFontCreate(pannel->dpy, pannel->win, fontname);
        XCBSetLineAttributes(display, pannel->gc, 1, XCB_LINE_STYLE_SOLID, XCB_CAP_STYLE_BUTT, XCB_JOIN_STYLE_MITER);
        return PannelInitLoop(pannel);
    }
    return pannel;
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
PannelDrawText(
        Pannel *pannel, 
        int32_t x, 
        int32_t y, 
        uint32_t len,
        char *str
        )
{
    PannelLock(pannel);
    if(pannel->font)
    {   XFFontDrawText(pannel->dpy, pannel->font, pannel->pix, x, y, len, (uint16_t *)str);
    }
    else
    {   DEBUG0("Failed to load font");
    }
    PannelUnlock(pannel);
}


static void
PannelDrawTitleScreen(Pannel *pannel)
{
    uint32_t red = 0xff0000;
    PannelLock(pannel);
    PannelDrawRectangle(pannel, 0, 0, pannel->w, pannel->h, 1, red);
    PannelUnlock(pannel);
}

static void
PannelDrawMoveButtons(Pannel *pannel)
{
    u32 red = 0xff0000;

    const u8 SEGMENTS_WIDTH = 10;
    const u8 SEGMENTS_HEIGHT = SEGMENTS_WIDTH * 2;
    const u8 SEGMENTS_WIDTH_HEIGHT = SEGMENTS_WIDTH * SEGMENTS_HEIGHT;
    const u8 NUM_BUTTONS = 3;

    enum
    {
        ___left,
        ___selected,
        ___right,
        __last,
    };

    const u8 NAME_LENS[__last] = 
    { 
        [___left] = sizeof("LEFT") - 1, 
        [___selected] = sizeof("SELECTED") - 1, 
        [___right] = sizeof("RIGHT") - 1
    };
    char *const NAMES[__last] = 
    { 
        [___left] = "LEFT", 
        [___selected] = "SELECTED", 
        [___right] = "RIGHT" 
    };

    const u32 ButtonW = pannel->w / SEGMENTS_WIDTH;
    const u32 ButtonH = pannel->h / SEGMENTS_WIDTH;

    const u32 heightpad = pannel->h / SEGMENTS_HEIGHT;
    const u32 middlepad = pannel->w / SEGMENTS_WIDTH_HEIGHT;

    const u32 GapsTotalWidth = (NUM_BUTTONS - 1) * middlepad;
    const u32 ButtonsTotalWidth = (NUM_BUTTONS * ButtonW);
    const u32 spacew = GapsTotalWidth + ButtonsTotalWidth;

    /* this just centers the buttons */
    const u32 leftpad = MAX((pannel->w - spacew) / 2, 0);

    u32 x = leftpad;
    /* make sure its at the bottom */
    u32 y = MAX(pannel->h - heightpad - ButtonH, 0);

    i32 i;
    for(i = 0; i < NUM_BUTTONS; ++i)
    {   
        PannelDrawRectangle(pannel, x, y, ButtonW, ButtonH, 1, red);
        PannelDrawText(pannel, x, y, strlen("SELECTED"), "SELECTED");
        PannelDrawBuff(pannel, x, y, pannel->w, pannel->h);
        XCBSync(pannel->dpy);
        x += ButtonW + middlepad;
    }
}

void
PannelRedraw(Pannel *pannel, int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    PannelLock(pannel);

    /* prevent artifacts from previous drawings */
    PannelClear(pannel);

    if(pannel->draw)
    {   pannel->draw(pannel);
    }
    PannelDrawMoveButtons(pannel);
    PannelDrawBuff(pannel, x, y, w, h);
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
        DEBUG0("Resized Pixmap");
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
    Pannel *pannel = malloc(sizeof(Pannel));
    pannel->w = w;
    pannel->h = h;
    pannel->win = parent;
    pannel->draw = NULL;
    if(PannelInitThreads(pannel))
    {
        free(pannel);
        return NULL;
    }
    else
    {
        pthread_t id;
        pthread_create(&id, NULL, PannelInit, pannel);
    }
    return pannel;
}

void
PannelClear(
        Pannel *pannel
        )
{
    PannelLock(pannel);
    PannelDrawRectangle(pannel, 0, 0, pannel->w, pannel->h, 1, 0);
    PannelUnlock(pannel);
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
    XCBCookie ret = XCBCopyArea(pannel->dpy, pannel->pix, pannel->win, pannel->gc, x, y, w, h, xoffset, yoffset);
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
