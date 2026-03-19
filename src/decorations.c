#include <stdlib.h>

#include "decorations.h"
#include "util.h"
#include "client.h"
#include "main.h"

extern WM _wm;

u32 ISDECORACTIVE(Client *c) { return c->decor && c->decor->holding; }

Decoration *
createdecoration(void)
{
    Decoration *decor = malloc(sizeof(*decor));

    if(decor)
    {
        decor->x = 0;
        decor->y = 0;
        decor->w = 1;
        decor->h = 1;
        decor->holding = 0;

        const u8 depth = XCB_COPY_FROM_PARENT;
        const XCBVisual visual = XCBDefaultVisual(_wm.dpy, _wm.screen);
        const u8  class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
        const u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT;

        XCBCreateWindowValueList va =
        {
            .background_pixel = ~0,
            .border_pixel = 0,
            .override_redirect = 0,
        };

        decor->win = XCBCreateWindow(_wm.dpy, _wm.root, 0, 0, decor->w, decor->h, 0, depth, class, visual, mask, &va);
    }

    return decor;
}

void 
decorationhold(Decoration *decor, Client *c)
{
    if(decor->holding)
    {   return;
    }

    decor->holding = 1;

    decorationupdate(decor, c);
    XCBMapWindow(_wm.dpy, decor->win);
    XCBReparentWindow(_wm.dpy, c->win, decor->win, 0, 0);
}

void
decorationrelease(Decoration *decor, Client *c)
{
    if(!decor->holding)
    {   return;
    }

    decor->holding = 0;

    XCBReparentWindow(_wm.dpy, c->win, _wm.root, c->x, c->y);
    XCBUnmapWindow(_wm.dpy, decor->win);
}

void 
decorationupdate(Decoration *decor, Client *c)
{
    if(!decor->holding)
    {   return;
    }

    u32 mask = 0;

    if(decor->x != c->x)
    {   
        decor->x = c->x;
        mask |= XCB_CONFIG_WINDOW_X;
    }

    if(decor->y != c->y)
    {
        decor->y = c->y;
        mask |= XCB_CONFIG_WINDOW_Y;
    }

    if(decor->w != c->w)
    {
        decor->w = c->w;
        mask |= XCB_CONFIG_WINDOW_WIDTH;
    }

    if(decor->h != c->h)
    {   
        decor->h = c->h;
        mask |= XCB_CONFIG_WINDOW_HEIGHT;
    }

    XCBWindowChanges changes =
    {   
        .x = decor->x,
        .y = decor->y,
        .width = decor->w,
        .height = decor->h,
    };

    if(mask)
    {   
        XCBConfigureWindow(_wm.dpy, decor->win, mask, &changes);
        mask &= ~(XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y);
        if(mask)
        {   XCBConfigureWindow(_wm.dpy, c->win, mask, &changes);
        }
    }
}

void
decorationfocus(Decoration *decor, Client *c, bool focus)
{
}

void 
cleanupdecoration(Decoration *decor)
{
    XCBDestroyWindow(_wm.dpy, decor->win);
    free(decor);
}
