#include <stdlib.h>

#include "decorations.h"
#include "util.h"

#define _DECOR_FLAGS_PREFER_CSD_    (1 << 0)

Decoration *
X11DecorCreate(void)
{
    Decoration *decor = malloc(sizeof(Decoration));
    if(decor)
    {
        decor->w = 1;
        decor->h = 1;
        decor->win = 0;
        decor->flags = 0;
    }
    return decor;
}

void 
X11DecorHoldChild(Decoration *decor, XCBWindow child)
{
    if(!decor)
    {   return;
    }
    decor->child = child;
}

void 
X11DecorMap(XCBDisplay *display, Decoration *decor)
{
    if(!display || !decor)
    {   return;
    }
    XCBMapWindow(display, decor->win);
}

void
X11DecorUnmap(XCBDisplay *display, Decoration *decor)
{
    if(!display || !decor)
    {   return;
    }
    XCBUnmapWindow(display, decor->win);
}

void 
X11DecorSetPreferCSD(Decoration *decor, uint8_t state)
{
    if(!decor)
    {   return;
    }
    const char *const GTK_CSD = "GTK_CSD";
    const char *const GTK_REPLACE = "1";

    SETFLAG(decor->flags, _DECOR_FLAGS_PREFER_CSD_, !!state);

    if(state)
    {   setenv(GTK_CSD, GTK_REPLACE, 1);
    }
    else
    {   unsetenv(GTK_CSD);
    }
}
