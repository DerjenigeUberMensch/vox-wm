#include "pannel.h"
#include "XCB-TRL/xcb_image.h"
#include "XCB-TRL/xcb_imgutil.h"
#include "dwm.h"

#include <stdlib.h>
#include <stdio.h>

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
    XCBDisplay *display,
    int32_t screen,
    XCBWindow parent, 
    int32_t x, 
    int32_t y, 
    int32_t w, 
    int32_t h
    )
{
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
        uint8_t *data
        )
{
    /* make sure coords are in possible space */
    x = MAX(x, widget->x);
    x = MIN(x, widget->x + widget->w);
    y = MAX(y, widget->y);
    y = MIN(y, widget->y + widget->h);

    /* handle too big values */
    if(w + x > widget->w)
    {   w -= x;
    }
    if(h + y > widget->h)
    {   h -= y;
    }

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
        uint8_t *colour
        )
{
    if(x * y <= pannel->w * pannel->h)
    {   
        uint32_t coordoffset = (y * pannel->w) + x;
        size_t byteoffset = coordoffset * pannel->itemsize;
        memcpy(((uint8_t *)pannel->buff) + (byteoffset), &colour, pannel->itemsize);
        return 0;
    }
    return 1;
}

int
PannelWriteBuff(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        uint16_t w,
        uint16_t h,
        uint8_t *data
        )
{
    /* make sure coords are in possible space */
    x = MAX(x, pannel->x);
    x = MIN(x, pannel->x + pannel->w);
    y = MAX(y, pannel->y);
    y = MIN(y, pannel->y + pannel->h);

    /* handle too big values */
    if(w + x > pannel->w)
    {   w -= x;
    }
    if(h + y > pannel->h)
    {   h -= y;
    }

    uint32_t coordoffset = (y * pannel->w) + x;
    uint32_t itemsize = pannel->itemsize;
    size_t byteoffset = coordoffset * itemsize;
    size_t size = itemsize * w * h;

    /* make sure its not empty/negative */
    if(size > 0)
    {   memcpy(((uint8_t *)pannel->buff) + (byteoffset),data, size);
        return 0;
    }
    return 1;
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
