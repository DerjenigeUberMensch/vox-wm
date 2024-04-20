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


