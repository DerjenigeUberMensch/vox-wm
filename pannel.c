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
        if(!pannel->buff)
        {   
            XCBDestroyWindow(pannel->dpy, pannel->win);
            XCBFreeGC(pannel->dpy, pannel->gc);
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
    XCBDestroyWindow(pannel->dpy, pannel->win);
    XCBFreeGC(pannel->dpy, pannel->gc);
    XCBFlush(pannel->dpy);
    XCBCloseDisplay(pannel->dpy);
    free(pannel->buff);
    for(i = 0 ; i < pannel->widgetslen; ++i)
    {   free((((PannelWidget *)pannel->widgets) + i)->buff);
    }
    free(pannel->widgets);
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
    return widget;
}

/* Resizes a widget based on parametors. Widget is assumed to be empty, as data WILL be destroyed.
 * 
 * RETURN: 1 on Failure.
 * RETURN: 0 on Success.
 */
int
PannelWidgetResize(
        PannelWidget *widget,
        uint16_t w,
        uint16_t h
        )
{
    if(w * h > widget->bufflength)
    {
        const u64 length = w * h;
        void *realoc = realloc(widget->buff, length * widget->itemsize);
        if(realoc)
        {   widget->buff = realoc;
            widget->bufflength = length;
        }
        else    /* dont change anything exit failure */
        {   return 1;
        }
    }
    widget->w = w;
    widget->h = h;
    return 0;
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
    VALUE_CLAMP_WIDGET(widget, &x, &y, &w, &h);
    uint32_t coordoffset = (y * widget->w) + x;
    uint32_t itemsize = widget->itemsize;
    size_t byteoffset = coordoffset * itemsize;
    size_t size = itemsize * w * h;

    /* make sure its not empty/negative */
    if(size > 0)
    {   memcpy(((uint8_t *)widget->buff) + (byteoffset), data, size);
    }
}

int
PannelWritePixel(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        void *colour
        )
{
    if(x * y < pannel->w * pannel->h)
    {   
        PLOT_PANNEL(pannel, x, y, colour);
        return 0;
    }
    return 1;
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
    VALUE_CLAMP_PANNEL(pannel, &x, &y, &w, &h);

    size_t size = pannel->itemsize * w * h;
    /* make sure its not empty/negative */
    if(size > 0)
    {   
        PLOT_PANNEL_POINTS(pannel, x, y, w, h, data);
        return 0;
    }
    return 1;
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

}

void
PannelWidgetDrawLine(
        PannelWidget *widget,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        void *colour
        )
{
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
}

void
PannelWidgetDrawRectangle(
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

    if(fill)
    {
        int64_t _y;
        for(_y = 0; _y < h; ++_y)
        {   PannelWidgetDrawLine(wi, x, _y + y, x + w, _y + y, colour);
        }
    }
    /* top */
    PannelWidgetDrawLine(wi, x, y, x + w, y, colour);
    /* bottom */
    PannelWidgetDrawLine(wi, x, y + h, x + w, y + h, colour);
    /* left side */
    PannelWidgetDrawLine(wi, x, y, x, y + h, colour);
    /* right side */
    PannelWidgetDrawLine(wi, x + w, y, x + w, y + h, colour);
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
    const unsigned int screen = pannel->screen;
    XCBDisplay *display = pannel->dpy;
    const XCBGC gc = pannel->gc;
    const XCBWindow win = pannel->win;
    const u8 format = XCB_IMAGE_FORMAT_BGRA;
    const u8 depth = XCBDefaultDepth(display, screen);
    const u8 left_pad = 0;
    const i32 x = pannel->x + xoffset;
    const i32 y = pannel->y + yoffset;
    const u32 size = w * h * sizeof(uint32_t);
    const uint8_t *data = (uint8_t *)pannel->buff;
    u32 i;
    for(i = 0; i < pannel->widgetslen; ++i)
    {   
        PannelWidget *widget = pannel->widgets + i;
        PLOT_PANNEL_POINTS(pannel, widget->x, widget->y, widget->w, widget->h, widget->buff);
    }
    XCBCookie ret = XCBPutPixels(display, gc, win, format, depth, left_pad, x, y, w, h, size, data);
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
