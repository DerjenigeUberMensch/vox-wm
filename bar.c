/*
 * NOT TO BE RELEASED ON FINAL BUILDS.
 *
 *
 *
 *
 * This is mostly for debugging certain values without clogging up gdb.
 * But can still be used as a status bar
 */


#include <stdio.h>
#include <stdarg.h>


#include "dwm.h"
extern WM _wm;

#include "bar.h"
#include "util.h"

void
writebar(char *fmt, ...)
{
    static FILE *bar = NULL;
    if(!bar)
    {   bar = popen("lemonbar -p", "w");
    }
    if(!bar)
    {   return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(bar, fmt, args);
    fprintf(bar, "\n");
    vfprintf(stdout, fmt, args);
    fflush(bar);
    va_end(args);
}

void
setshowbar(Bar *bar, uint8_t state)
{
    SETFLAG(bar->flags, _SHOW_BAR, !!state);
}

void
resizebar(Bar *bar, int32_t x, int32_t y, int32_t w, int32_t h)
{
    if(x != bar->x || y != bar->y)
    {   XCBMoveWindow(_wm.dpy, bar->win, x, y);
    }
    if(w != bar->w || h != bar->h)
    {   XCBResizeWindow(_wm.dpy, bar->win, w, h);
    }
    bar->x = x;
    bar->y = y;
    bar->w = w;
    bar->h = h;

    const XCBConfigureNotifyEvent ce = 
    {
        .response_type = XCB_CONFIGURE_NOTIFY,
        .event = bar->win,
        .window = bar->win,
        .x = bar->x,
        .y = bar->y,
        .width = bar->w,
        .height = bar->h,
        .border_width = 0,
        .above_sibling = XCB_NONE,
        .override_redirect = False,
    };

    /* valgrind says that this generates some stack allocation error in writev(vector[1]) but it seems to be a xcb issue */
    XCBSendEvent(_wm.dpy, bar->win, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ce);
}

/*
 * NOTE: No side effects if resize fails.
 *
 * RETURN: 0 On Success.
 * RETURN: 1 On Failure.
 */
int
BarResizeBuff(
        Bar *bar,
        uint32_t w,
        uint32_t h
        )
{
    XCBPixmap pixmap = bar->pix;
    XCBDisplay *display = bar->dpy;
    unsigned int screen = bar->screen;
    if(pixmap)
    {
        if(bar->w < w || bar->h < h)
        {   
            const size_t newsize = sizeof(char) * w * h;
            XCBFreePixmap(display, pixmap);
            bar->pix = XCBCreatePixmap(display, XCBRootWindow(display, screen), w, h, XCBDefaultDepth(display, screen));
            char *buff = realloc(bar->writebuff, newsize);
            if(buff)
            {   
                bar->writebuff = buff;
                bar->buffsize = newsize;
            }
            return !buff;
        }
    }
    return 1;
}


/* Builtin bar Stuff */

Bar *
BarCreate(
    XCBDisplay *display,
    int32_t screen,
    XCBWindow parent, 
    int32_t x, 
    int32_t y, 
    int32_t w, 
    int32_t h
    )
{
    Bar *bar = malloc(sizeof(Bar));
    const uint8_t red = 0;
    const uint8_t green = 0;
    const uint8_t blue = 0;
    const uint8_t alpha = 0;    /* transparency, 0 is opaque, 255 is fully transparent */
    const uint32_t bordercolor = blue + (green << 8) + (red << 16) + (alpha << 24);
    const uint32_t backgroundcolor = bordercolor;
    const uint32_t borderwidth = 0;

    if(bar)
    {
        bar->x = x;
        bar->y = y;
        bar->w = w;
        bar->h = h;
        bar->win = XCBCreateSimpleWindow(display, parent, x, y, w, h, borderwidth, bordercolor, backgroundcolor);
        bar->pix = XCBCreatePixmap(display, XCBRootWindow(display, screen), w, h, XCBDefaultDepth(display, screen));
        bar->gc = XCBCreateGC(display, XCBRootWindow(display, screen), 0, NULL);
        bar->dpy = display;
        bar->screen = screen;
        bar->buffsize = sizeof(char) * w * h;
        bar->writebuff = calloc(1, bar->buffsize);
        XCBSetLineAttributes(display, bar->gc, 1, XCB_LINE_STYLE_SOLID, XCB_CAP_STYLE_BUTT, XCB_JOIN_STYLE_MITER);
        if(!bar->writebuff)
        {   
            XCBDestroyWindow(bar->dpy, bar->win);
            XCBFreePixmap(bar->dpy, bar->pix);
            XCBFreeGC(bar->dpy, bar->pix);
            free(bar);
            bar = NULL;
        }
    }
    return bar;
}

void
BarDestroy(
    Bar *bar
        )
{
    XCBDestroyWindow(bar->dpy, bar->win);
    XCBFreePixmap(bar->dpy, bar->pix);
    XCBFreeGC(bar->dpy, bar->pix);
    free(bar->writebuff);
    free(bar);   
}

void
BarWriteBuff(
    Bar *bar, 
    size_t byteoffset, 
    size_t size, 
    char *datatowrite
    )
{
    /* CLAMP */
    byteoffset = MIN(byteoffset, bar->buffsize - sizeof(char));
    byteoffset = MAX(byteoffset, 0);
    size = MIN(size, bar->buffsize - byteoffset);
    size = MAX(byteoffset, sizeof(char));

    char *buffoffset = (char *)bar->writebuff + byteoffset;
    memcpy(buffoffset, datatowrite, size);
}

/* TOD */
XCBCookie
BarDrawBuff(
    Bar *bar,
    int16_t xoffset,
    int16_t yoffset,
    const uint16_t w,
    const uint16_t h
        )
{
    char *buff = (char *)bar->writebuff;
    XCBCookie x = { .sequence = 0 };
    (void)x;
    (void)buff;
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
