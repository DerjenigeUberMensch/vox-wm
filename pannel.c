#include "pannel.h"

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
    XCBPixmap pixmap = pannel->pix;
    XCBDisplay *display = pannel->dpy;
    unsigned int screen = pannel->screen;
    if(pixmap)
    {
        if(pannel->w < w || pannel->h < h)
        {   
            const uint32_t newsize = pannel->itemsize * w * h;
            XCBFreePixmap(display, pixmap);
            pannel->pix = XCBCreatePixmap(display, XCBRootWindow(display, screen), w, h, XCBDefaultDepth(display, screen));
            char *buff = realloc(pannel->buff, newsize);
            if(buff)
            {   
                pannel->buff = buff;
                pannel->bufflength = w * h;
            }
            return !buff;
        }
    }
    return 1;
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
    const uint8_t red = 50;
    const uint8_t green = 5;
    const uint8_t blue = 10;
    const uint8_t alpha = 0;    /* transparency, 0 is opaque, 255 is fully transparent */
    const uint32_t bordercolor = blue + (green << 8) + (red << 16) + (alpha << 24);
    const uint32_t backgroundcolor = bordercolor;
    const uint32_t borderwidth = 0;

    if(pannel)
    {
        pannel->x = x;
        pannel->y = y;
        pannel->w = w;
        pannel->h = h;
        pannel->win = XCBCreateSimpleWindow(display, parent, x, y, w, h, borderwidth, bordercolor, backgroundcolor);
        pannel->pix = XCBCreatePixmap(display, XCBRootWindow(display, screen), w, h, XCBDefaultDepth(display, screen));
        pannel->gc = XCBCreateGC(display, XCBRootWindow(display, screen), 0, NULL);
        pannel->dpy = display;
        pannel->screen = screen;
        pannel->itemsize = sizeof(uint32_t);
        pannel->bufflength = w * h;
        pannel->buff = calloc(1, pannel->bufflength * pannel->itemsize);
        XCBSetLineAttributes(display, pannel->gc, 1, XCB_LINE_STYLE_SOLID, XCB_CAP_STYLE_BUTT, XCB_JOIN_STYLE_MITER);
        if(!pannel->buff)
        {   
            XCBDestroyWindow(pannel->dpy, pannel->win);
            XCBFreePixmap(pannel->dpy, pannel->pix);
            XCBFreeGC(pannel->dpy, pannel->pix);
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
    XCBDestroyWindow(pannel->dpy, pannel->win);
    XCBFreePixmap(pannel->dpy, pannel->pix);
    XCBFreeGC(pannel->dpy, pannel->pix);
    free(pannel->buff);
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
    PannelWidget *ret = malloc(sizeof(PannelWidget));
    if(ret)
    {
        ret->x = x;
        ret->y = y;
        ret->w = w;
        ret->h = h;
        ret->itemsize = pannel->itemsize;
        ret->bufflength = w * h;
        ret->buff = malloc(ret->bufflength * ret->itemsize);
    }
    return ret;
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
    const uint32_t widgetlen = widget->w * widget->h;
    const uint32_t newlen = w * h;
    const size_t widgetitemsize = widget->itemsize;
    const size_t pannelitemsize = pannel->itemsize;

    const size_t widgetsize = widgetlen * widgetitemsize;
    const size_t newsize = newlen * widgetlen;

    char *realoc = NULL;
    /* replace old size */
    if(newsize > widgetsize)
    {   
        realoc = realloc(widget->buff, newsize);
        if(realoc)
        {   widget->buff = realoc;
        }
    }
    if(widgetitemsize != pannelitemsize)
    {   
        widget->bufflength = newsize / pannelitemsize;
        widget->itemsize = pannelitemsize;
    }
    return !realoc;
}

/* Copies data to widget buffer using specified coordinates,
 * char *buffcopy is simply char * to get constant size, 
 * real data is interpreted by user.
 *
 * RETURN: 1 on Failure.
 * RETURN: 0 on Success.
 */
int
PannelWidgetWrite(
        Pannel *pannel,
        PannelWidget *widget,
        char *buffcopy,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        )
{
    uint32_t initialoffset = x + y;
    uint32_t actualsize = w * h;
    if(initialoffset + actualsize > widget->bufflength)
    {   int status = PannelWidgetResize(pannel, widget, w, h);
        if(status)
        {   return 1;
        }
    }
    memcpy(((char *)widget->buff) + (initialoffset * widget->itemsize), buffcopy, actualsize * widget->itemsize);
    return 0;
}

void
PannelFlush(Pannel *pannel)
{
}

void
PannelDrawRect(
        Pannel *pannel, 
        int32_t x, 
        int32_t y, 
        uint32_t w, 
        uint32_t h,
        uint8_t fill,
        uint32_t bordercol,
        uint32_t fillcol
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
    XCBCookie x = { .sequence = 0 };
    xcb_put_image(pannel->dpy, XCB_IMAGE_FORMAT_Z_PIXMAP, pannel->pix, pannel->gc, w, h, 0, 0, 0, XCBDefaultDepth(pannel->dpy, pannel->screen), pannel->w * pannel->h * 4, (unsigned char *)pannel->buff);
    XCBFlush(pannel->dpy);
    XCBCopyArea(pannel->dpy, pannel->pix, pannel->win, pannel->gc, 0, 0, pannel->w, pannel->h, pannel->x, pannel->y);
    return x;
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
