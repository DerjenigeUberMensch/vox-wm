#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <signal.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
#include <xcb/xkb.h>
#include <xcb/xinerama.h>

#include <X11/X.h> /* error codes */

/* keycodes */
#include <X11/keysym.h>

#include "xcb_trl.h"
#include "xcb_winutil.h"
#include "util.h"
#include "events.h"
#include "dwm.h"

#include "config.h"
#include "keybinds.h"



extern void (*handler[]) (XCBGenericEvent *);

WM _wm;
CFG _cfg;

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];

void
argcvhandler(int argc, char *argv[])
{
    int i;
    for(i = 0; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-h"))
        {
            printf( "Usage: dwm [options]\n"
                    "  -h           Help Information.\n"
                    "  -v           Compiler Information.\n"
                    );
            exit(EXIT_SUCCESS);
        }
        else if (!strcmp(argv[i], "-v"))
        {
            char *compiler = "UNKNOWN";
            i16 majorversion = -1;
            i16 minorversion = -1;
            i16 patchversion = -1;
            #if __GNUC__
            compiler = "GCC";
            majorversion = __GNUC__;
            minorversion = __GNUC_MINOR__;
            patchversion = __GNUC_PATCHLEVEL__;
            #endif
            #if __clang__
            compiler = "CLANG";
            majorversion = __clang_major__;
            minorversion = __clang_minor__;
            patchversion = __clang_patchlevel__;
            #endif
            printf( "Compiler Information.\n"
                    "  Compiled:        %s %s\n"
                    "  Compiler:        [%s v%d.%d.%d]\n" 
                    "  STDC:            [%d] [%lu]\n"
                    "  BYTE_ORDER:      [%d]\n"
                    "  POINTER_SIZE:    [%d]\n"
                    "Version Information.\n"
                    "  VERSION:         [%s]\n"
                    "  MARK:            [%s]\n"
                    ,
                   /* TODO __DATE__ has an extra space for some reason? */ 
                    __DATE__, __TIME__,
                    compiler, majorversion, minorversion, patchversion,
                    __STDC_HOSTED__, __STDC_VERSION__,
                    __BYTE_ORDER__,
                    __SIZEOF_POINTER__,
                    VERSION,
                    NAME
                    );
            exit(EXIT_SUCCESS);
        }
        else
        {   
            const char exec1 = '.';
            const char exec2 = '/';
            const char execcount = 3; /* not 2 because we need \0 */ /* +1 for the possible 1 letter name and +1 again for \0   */
            if(argv[0] != NULL && strnlen(argv[0], execcount + 2) >= execcount && argv[0][0] == exec1 && argv[0][1] == exec2)
            {   
                /* We can call die because it is very likely this was run manually */
                if(i > 0)
                {
                    printf("%s%s%s", "UNKNOWN COMMAND: '", argv[i], "'\n");
                    printf( "Usage: dwm [options]\n"
                            "  -h           Help Information.\n"
                            "  -v           Compiler Information.\n"
                          );
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {
                /* We dont die because likely this command was run using some form of exec */
                printf("%s%s%s", "UNKNOWN COMMAND: '", argv[i], "'\n");
            }
        }
    }
}

/* These are mostly user operations type deal */
void
applysizechecks(Monitor *m, i32 *x, i32 *y, i32 *width, i32 *height, i32 *border_width)
{
    i16 wx = m->wx;     /* Window Area X */
    i16 wy = m->wy;     /* Window Area Y */
    u16 ww = m->ww;     /* Window Area W */
    u16 wh = m->wh;     /* Window Area H */

        /* technically 1 is "too" small for most possible windows (they stay at roughly ~20 or higher) but a window would request this */
    const u8 MIN_POSSIBLE_WINDOW_SIZE = 1;
    const u8 NO_BORDER_WIDTH = 0;

    /* I was bored lol */

    /* if width less than min set to min */
    *width *= *width > MIN_POSSIBLE_WINDOW_SIZE;
    *width += !(*width) * MIN_POSSIBLE_WINDOW_SIZE;

    /* if width more than max set to max */
    *width *= *width < ww;
    *width += !(*width) * ww;

    /* if height less than min to set to min */
    *height *= *height > MIN_POSSIBLE_WINDOW_SIZE;
    *height += !(*height) * MIN_POSSIBLE_WINDOW_SIZE;

    /* if height more than max to set to max */
    *height *= *height < wh;
    *height += !(*height) * wh;


    /* if border_width more than min to set to min */
    *border_width *= *border_width > NO_BORDER_WIDTH;
    *border_width += !(*border_width) * NO_BORDER_WIDTH;

    /* if border_width more than max to set to max */
    *border_width *= *border_width < (ww - MIN_POSSIBLE_WINDOW_SIZE);
    *border_width += !(*border_width) * (ww - MIN_POSSIBLE_WINDOW_SIZE);

    /* if x less than min set to min */
    *x *= *x > (wx - ww);
    *x += !(*x) * (wx - ww);

    /* if x more than max set to max */
    *x *= *x < (wx + ww);
    *x += !(*x) * (wx + ww);


    /* if y less than min set to min */
    *y *= *y > (wy - wh);
    *y += !(*y) * (wy - wh);

    /* if y more than max set to max */
    *y *= *y < (wy + wh);
    *y += !(*y) * (wy + wh);
}

void
applygravity(u32 gravity, i16 *x, i16 *y, const u16 w, const u16 h, const u16 bw)
{
    if(!gravity || !x || !y)
    {   return;
    }
    /* This is bullshit just reference relative to this point */
    if(gravity & XCB_GRAVITY_STATIC)
    {   /* default do nothing */
    }
    else if(gravity & XCB_GRAVITY_NORTH_WEST)
    {
        *x -= bw;
        *y -= bw;
    }
    else if(gravity & XCB_GRAVITY_NORTH)
    {   
        *x += w >> 1;
        *y -= bw;
    }
    else if(gravity & XCB_GRAVITY_NORTH_EAST)
    {
        *x += w + bw;
        *y -= bw;
    }
    else if(gravity & XCB_GRAVITY_EAST)
    {
        *x += w + bw;
        *y += h >> 1;
    }
    else if(gravity & XCB_GRAVITY_SOUTH_EAST)
    {
        *x += w + bw;
        *y += h + bw;
    }
    else if(gravity & XCB_GRAVITY_SOUTH)
    {
        *x += w >> 1;
        *y += h + bw;
    }
    else if(gravity & XCB_GRAVITY_SOUTH_WEST)
    {
        *x -= bw;
        *y += h + bw;
    }
    else if(gravity & XCB_GRAVITY_WEST)
    {
        *x -= bw;
        *y += h >> 1;
    }
    else if(gravity & XCB_GRAVITY_CENTER)
    {
        *x += w >> 1;
        *y += h >> 1;
    }
}

uint8_t
applysizehints(Client *c, i32 *x, i32 *y, i32 *width, i32 *height, uint8_t interact)
{
    u8 baseismin;
    const Monitor *m = c->desktop->mon;
    XCBCookie sizehintscookie;
    if(ISFLOATING(c))
    {   sizehintscookie = XCBGetWMNormalHintsCookie(_wm.dpy, c->win);
    }

    /* set minimum possible */
    *width = MAX(1, *width);
    *height = MAX(1, *height);
    /* set max possible */
    *width = MIN(m->mw, *width);
    *height = MIN(m->mh, *height);

    if (interact)
    {
        if (*x > _wm.sw) 
        {   *x = _wm.sw - WIDTH(c);
        }
        if (*y > _wm.sh) 
        {   *y = _wm.sh - HEIGHT(c);
        }
        if (*x + *width + (c->bw << 1) < 0)
        {   *x = 0;
        }
        if (*y + *height + (c->bw << 1) < 0)
        {   *y = 0;
        }
    }
    else
    {
        if (*x >= m->wx + m->ww)
        {   *x = m->wx + m->ww - WIDTH(c);
        }
        if (*y >= m->wy + m->wh) 
        {   *y = m->wy + m->wh - HEIGHT(c);
        }
        if (*x + *width + (c->bw << 1) <= m->wx) 
        {   *x = m->wx;
        }
        if (*y + *height + (c->bw << 1) <= m->wy) 
        {   *y = m->wy;
        }
    }

    if (ISFLOATING(c))
    {
        XCBSizeHints hints;
        u8 status = XCBGetWMNormalHintsReply(_wm.dpy, sizehintscookie, &hints);
        /* On Failure clear flag and ignore hints */
        hints.flags *= !!status;
        updatesizehints(c, &hints);
        /* see last two sentences in ICCCM 4.1.2.3 */
        baseismin = c->basew == c->minw && c->baseh == c->minh;
        /* temporarily remove base dimensions */
        if (!baseismin)
        {
            *width  -= c->basew;
            *height -= c->baseh;
        }
        /* adjust for aspect limits */
        if (c->mina > 0 && c->maxa > 0)
        {
            if (c->maxa < (float)*width / *height) 
            {   *width = *height * c->maxa + 0.5;
            }
            else if (c->mina < (float)*height / *width) 
            {   *height = *width * c->mina + 0.5;
            }
        }
        /* increment calculation requires this */
        if (baseismin)
        {
            *width  -= c->basew;
            *height -= c->baseh;
        }
        /* adjust for increment value */
        if (c->incw)
        {   *width -= *width % c->incw;
        }
        if (c->inch) 
        {   *height -= *height % c->inch;
        }
        /* restore base dimensions */
        *width = MAX(*width + c->basew, c->minw);
        *height = MAX(*height + c->baseh, c->minh);
        if (c->maxw) 
        {   *width = MIN(*width, c->maxw);
        }
        if (c->maxh) 
        {   *height = MIN(*height, c->maxh);
        }
    }
    return *x != c->x || *y != c->y || *width != c->w || *height != c->h;
}

void
arrange(Desktop *desk)
{
    Client *c;
    static Desktop *workingdesk = NULL;

    if(workingdesk != desk)
    {
        for(c = desk->stack; c; c = nextstack(c))
        {   /* configuring windows is suprisingly expensive */
            showhide(c);
        }
        workingdesk = desk;
    }

    arrangedesktop(desk);
    restack(desk);
}

void
arrangemon(Monitor *m)
{
    Desktop *desk;
    for(desk = m->desktops; desk; desk = nextdesktop(desk))
    {   arrange(desk);
    }
}


void
arrangemons(void)
{
    Monitor *m;
    for(m = _wm.mons; m; m = nextmonitor(m))
    {   arrangemon(m);
    }
}

void
arrangedesktop(Desktop *desk)
{
    layouts[desk->layout].arrange(desk);
    /* update the bar or something */
}

void
attach(Client *c)
{
    c->next = c->desktop->clients;
    c->desktop->clients = c;
    if(c->next)
    {   c->next->prev = c;
    }
    else
    {   c->desktop->clast = c;
    }
    /* prevent circular linked list */
    c->prev = NULL;
}

void
attachdesktop(Monitor *m, Desktop *desktop)
{
    desktop->mon = m;
    desktop->next = m->desktops;
    m->desktops = desktop;
    if(desktop->next)
    {   desktop->next->prev = desktop;
    }
    else
    {  
         /* m->lastdesktop = desktop */
    }
    /* prevent circular linked list */
    desktop->prev = NULL;
    ++m->deskcount;
}

void
attachstack(Client *c)
{
    Desktop *desk = c->desktop;
    c->snext = desk->stack;
    desk->stack = c;
    if(c->snext)
    {   
        c->snext->sprev = c;
    }
    else
    {   
        desk->slast = c;
    }
    /* prevent dangling pointers */
    c->sprev = NULL;
}

void
detach(Client *c)
{
    Client **tc;
    for (tc = &c->desktop->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
    if(!(*tc)) 
    {
        c->desktop->clast = c->prev;
    }
    else if(c->next) 
    {   c->next->prev = c->prev;
    }
    else if(c->prev) 
    {   /* This should be UNREACHABLE but in case we do reach it then this should suffice*/
        c->desktop->clast = c->prev;
        c->prev->next = NULL;
    }
    c->next = NULL;
    c->prev = NULL;
}

void
detachcompletely(Client *c)
{
    detach(c);
    detachstack(c);
}

void
detachdesktop(Monitor *m, Desktop *desktop)
{
    Desktop **td;
    for(td = &m->desktops; *td && *td != desktop; td = &(*td)->next);
    *td = desktop->next;
    if(!(*td))
    {
        /* m->lastdesktop = desktop->prev; */
    }
    else if(desktop->next)
    {
        desktop->next->prev = desktop->prev;
    }
    else if(desktop->prev)
    {
        /* m->last = desktop->prev; */
        desktop->prev->next = NULL;
    }
    --m->deskcount;

    desktop->next = NULL;
    desktop->prev = NULL;
    desktop->mon = NULL;
}

void
detachstack(Client *c)
{
    Desktop *desk = c->desktop;
    Client **tc, *t;

    for(tc = &desk->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;
    if(!(*tc))
    {
        desk->slast = c->sprev;
    }
    else if(c->snext)
    {   
        c->snext->sprev = c->sprev;
    }
    else if(c->sprev)
    {   /* this should be UNREACHABLE but if it does this should suffice */
        desk->slast = c->sprev;
        c->sprev->snext = NULL;
    }
    if(c == desk->sel)
    {
        for(t = desk->stack; t && !ISVISIBLE(t); t = t->snext);
        desk->sel = t;
    }
    c->sprev = NULL;
    c->snext = NULL;
}

u8
checknewbar(int64_t value[12], XCBAtom wtypes[], uint32_t windowtypeslength, XCBAtom wmstates[], uint32_t wmstateslength, int64_t desktop)
{
    u8 isbar  = 0;
    u8 sticky = 0;
    u8 strut  = 0;
    u8 isdock = 0;
    u8 skippager = 0;
    u8 above = 0;

    /* yeah idk */
    sticky = !!(desktop & 0xFFFFFFFF) || (uint32_t)desktop == (uint32_t)~0 || (int32_t)desktop == -1;
    sticky = sticky || (uint32_t)desktop == (uint32_t) -1 || (uint32_t)desktop == UINT32_MAX;

    const i32 left = value[0];
    const i32 right = value[1];
    const i32 top = value[2];
    const i32 bottom = value[3];
    const i32 left_start_y = value[4];
    const i32 left_end_y = value[5];
    const i32 right_start_y = value[6];
    const i32 right_end_y = value[7];
    const i32 top_start_x = value[8];
    const i32 top_end_x = value[9];
    const i32 bottom_start_x = value[10];
    const i32 bottom_end_x = value[11];

    strut = left_start_y || left_end_y || right_start_y || right_end_y || top_start_x || top_end_x;
    strut = strut || left || right || top || bottom;

    u32 i;
    for(i = 0; i < windowtypeslength; ++i)   
    {
        if(wtypes[i] == netatom[NetWMWindowTypeNormal])
        {   
            isdock = 0;
            break;
        }
        isdock = isdock || wtypes[i] == netatom[NetWMWindowTypeDock] || wtypes[i] == netatom[NetWMWindowTypeToolbar];
    }

    for(i = 0; i < wmstateslength; ++i)
    {
        sticky = sticky || wmstates[i] == netatom[NetWMStateSticky];
        skippager = skippager || wmstates[i] == netatom[NetWMStateSkipPager];
        above = above || wmstates[i] == netatom[NetWMStateAlwaysOnTop];
    }

    if(sticky && strut)
    {   isbar = 1;
    }
    else if(strut && above)
    {   isbar = 1;
    }
    else if(strut && isdock)
    {   isbar = 1;
    }
    else if(isdock && above && sticky)
    {   isbar = 1;
    }
    else if(above && isdock && skippager)
    {   isbar = 1;
    }

    return isbar;
}

void
checkotherwm(void)
{
    XCBGenericEvent *ev = NULL;
    i32 response;
    XCBSelectInput(_wm.dpy, XCBRootWindow(_wm.dpy, _wm.screen), XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
    XCBSync(_wm.dpy);  /* XCBFlush has different behaviour suprisingly, its undesired though */
    /* XCBPollForEvent calls the XServer itself for the event, So if we get a reply then a type of Window manager must be running */
    if((ev = XCBPollForEvent(_wm.dpy)))
    {   
        response = ev->response_type;
        free(ev);
        XCBCloseDisplay(_wm.dpy);
        if(response == 0) 
        {   DIECAT("%s", "FATAL: ANOTHER WINDOW MANAGER IS RUNNING.");
        }
        /* UNREACHABLE */
        DIECAT("%s", "FATAL: UNKNOWN REPONSE_TYPE");
    }
    /* assuming this isnt a bug we received nothing because we pinged the server for a response and got nothing */
    /* The other edge case is if the display just doesnt work, however this is covered at startup() if(!_wm.dpy) { DIE(msg); } */
}

uint8_t
checksticky(int64_t x)
{
    /* _NET_WM_DESKTOP
     * https://specifications.freedesktop.org/wm-spec/latest/
     */
    return (x & 0xFFFFFFFF) | ((uint32_t)x == UINT32_MAX) | ((unsigned int)x == ~0);
}

void
cleanup(void)
{
    if(!_wm.dpy)
    {
        /* sometimes due to our own lack of competence we can call quit twice and segfault here */
        if(_wm.selmon)
        {
            DEBUG0("Some data has not been freed exiting due to possible segfault.");
        }
        return;
    }
    XCBCookie cookie = XCBDestroyWindow(_wm.dpy, _wm.wmcheckwin);
    XCBDiscardReply(_wm.dpy, cookie);
    _wm.wmcheckwin = 0;
    if(_wm.syms)
    {   
        XCBKeySymbolsFree(_wm.syms);
        _wm.syms = NULL;
    }
    cleanupmons();
    XCBFlush(_wm.dpy);

    if(_wm.dpy)
    {
        XCBCloseDisplay(_wm.dpy);
        _wm.dpy = NULL;
    }
    ToggleExit();
}

void
cleanupdesktop(Desktop *desk)
{
    Client *c = NULL;
    Client *next = NULL;
    c = desk->clients;
    while(c)
    {   
        next = c->next;
        cleanupclient(c);
        c = next;
    }
    free(desk);
    desk = NULL;
}

void
cleanupclient(Client *c)
{
    free(c);
    c = NULL;
}

void
cleanupmon(Monitor *m)
{
    Desktop *desk = NULL;
    Desktop *desknext = NULL;
    desk = m->desktops;
    while(desk)
    {
        desknext = desk->next;
        cleanupdesktop(desk);
        desk = desknext;
    }
    free(m->bar);
    free(m);
    m = NULL;
}

void
cleanupmons(void)
{
    Monitor *m = NULL;
    Monitor *mnext = NULL;
    m = _wm.mons;

    while(m)
    {   
        mnext = m->next;
        cleanupmon(m);
        m = mnext;
    }
}

void
configure(Client *c)
{
    const XCBConfigureNotifyEvent ce = 
    {
        .response_type = XCB_CONFIGURE_NOTIFY,
        .event = c->win,
        .window = c->win,
        .x = c->x,
        .y = c->y,
        .width = c->w,
        .height = c->h,
        .border_width = c->bw,
        .above_sibling = XCB_NONE,
        .override_redirect = False,
    };
    /* valgrind says that this generates some stack allocation error in writev(vector[1]) but it seems to be a xcb issue */
    XCBSendEvent(_wm.dpy, c->win, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ce);
}

Client *
createclient(void)
{
    Client *c = calloc(1, sizeof(Client ));
    if(!c)
    {   DEBUG0("Could not allocate memory for client (OutOfMemory).");
        return NULL;
    }
    c->x = c->y = 0;
    c->w = c->h = 0;
    c->oldx = c->oldy = 0;
    c->oldw = c->oldh = 0;
    c->wtypeflags = 0;
    c->wstateflags = 0;
    c->bw = c->oldbw = 0;
    c->bcol = 0;
    c->win = 0;
    c->mina = c->maxa = 0;
    c->basew = c->baseh = 0;
    c->incw = c->inch = 0;
    c->maxw = c->maxh = 0;
    c->pid = 0;
    c->desktop = NULL;
    return c;
}

Bar *
createbar(void)
{
    Bar *bar = ecalloc(1, sizeof(Bar));

    if(!bar)
    {   /* bar is not important enough to stop operation. */
        DEBUG0("Failed to init bar.");
        return NULL;
    }

    bar->x = 0;
    bar->y = 0;
    bar->h = 0;
    bar->w = 0;
    bar->win = 0;
    bar->flags = 0;
    return bar;
}

Desktop *
createdesktop(void)
{
    Desktop *desk = calloc(1, sizeof(Desktop));
    if(!desk)
    {
        DEBUG("%s", "WARN: FAILED TO CREATE DESKTOP");
        return NULL;
    }
    desk->layout = 0;   /* TODO */
    desk->olayout= 0;   /* TODO */
    desk->clients= NULL;
    desk->stack = NULL;
    desk->mon = NULL;
    return desk;
}

Monitor *
createmon(void)
{
    Monitor *m = calloc(1, sizeof(Monitor ));
    if(!m)
    {   /* while calling "DIE" may seem extreme frankly we cannot afford a monitor to fail alloc. */
        DIE("%s", "(OutOfMemory) Could not alloc enough memory for a Monitor");
    }
    m->mx = m->my = 0;
    m->mw = m->mh = 0;
    m->wx = m->wy = 0;
    m->ww = m->wh = 0;
    m->next = NULL;
    u16 i;
    /* TODO */
    Desktop *desk;
    for(i = 0; i < 10; ++i)
    {   
        desk = createdesktop();
        attachdesktop(m, desk);
    }
    m->desksel = m->desktops;
    m->bar = calloc(1, sizeof(Bar ));
    if(!m->bar)
    {   /* we dont care too much here if we fail to alloc memory */
        DEBUG0("(OutOfMemory) Failed to create bar.");
    }
    return m;
}

Monitor *
dirtomon(u8 dir)
{
    Monitor *m = NULL;

    if(dir > 0)
    {   if(!(m = _wm.selmon->next)) m = _wm.mons;
    }
    else if (_wm.selmon == _wm.mons)
    {
        for(m = _wm.mons; m->next; m = nextmonitor(m));
    }
    else
    {   for(m = _wm.mons; m->next != _wm.selmon; m = nextmonitor(m));
    }
    return m;
}

/* unused. */
uint8_t
docked(Client *c)
{
    const Monitor *m = c->desktop->mon;
    uint8_t dockd = 0;
    if(m)
    {   
        dockd = (m->wx == c->x) & (m->wy == c->y) & (WIDTH(c) == m->ww) & (HEIGHT(c) == m->wh);
    }
    return dockd;
}

void
eventhandler(XCBGenericEvent *ev)
{
    /* int for speed */
    int cleanev = XCB_EVENT_RESPONSE_TYPE(ev);
    DEBUG("%s", XCBGetEventName(cleanev));
    if(handler[cleanev])
    {   handler[cleanev](ev);
    }
}

void
exithandler(void)
{   
    DEBUG("%s", "Process Terminated Successfully.");
}

void
floating(Desktop *desk)
{
    /* for now just check in restack for it */
    monocle(desk);
}

void
focus(Client *c)
{
    Monitor *selmon = _wm.selmon;
    Desktop *desk  = selmon->desksel;
    if(!c || !ISVISIBLE(c))
    {   /* selmon should have atleast 1 desktop so no need to check */
        for(c = desk->stack; c && !ISVISIBLE(c); c = c->snext);
    }
    if(desk->sel && desk->sel != c)
    {   unfocus(desk->sel, 0);
    }
    if(c)
    {  

        if(c->desktop->mon != _wm.selmon)
        {   _wm.selmon = c->desktop->mon;
        }

        if(ISURGENT(c))
        {   seturgent(c, 0);
        }
        /* make it first on the stack */
        detachstack(c);
        attachstack(c);
        grabbuttons(c->win, 1);
        /* for some reason changin focus removes border, but doesnt remove the width bugfix TODO */
        XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);
        if(c->desktop->lastfocused && c->desktop->lastfocused != c)
        {   XCBSetWindowBorder(_wm.dpy, c->desktop->lastfocused->win, c->desktop->lastfocused->bcol);
        }
        setfocus(c);
    }
    else
    {   
        XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
    }
    desk->sel = c;
    DEBUG("Focused: [%d]", c ? c->win : 0);
}

i8
getrootptr(i16 *x, i16 *y)
{
    u8 samescr;

    XCBCookie cookie = XCBQueryPointerCookie(_wm.dpy, _wm.root);
    XCBQueryPointer *reply = XCBQueryPointerReply(_wm.dpy, cookie);

    if(!reply)
    {   return 0;
    }

    *x = reply->root_x;
    *y = reply->root_y;
    samescr = reply->same_screen;
    free(reply);
    return samescr;
}

void
grabbuttons(XCBWindow win, uint8_t focused)
{
    u16 i, j;
    /* numlock is int */
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK};
    XCBUngrabButton(_wm.dpy, XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY, win);
    if (!focused)
    {
        XCBGrabButton(_wm.dpy, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY, win, 0, BUTTONMASK, 
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE);
    }
    for (i = 0; i < LENGTH(buttons); i++)
    {
        for (j = 0; j < LENGTH(modifiers); j++)
        {
            XCBGrabButton(_wm.dpy, buttons[i].button, 
                    buttons[i].mask | modifiers[j], 
                    win, 0, BUTTONMASK, 
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, 
                    XCB_NONE, XCB_NONE);
            DEBUG("Grabbed button: [%d]", buttons[i].button);
        }
    }
}

void
grabkeys(void)
{
    u32 i, j, k;
    u32 modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK };
    XCBKeyCode *keycodes[LENGTH(keys)];
    XCBUngrabKey(_wm.dpy, XCB_GRAB_ANY, XCB_MOD_MASK_ANY, _wm.root);
    
    /* This grabs all the keys */
    for(i = 0; i < LENGTH(keys); ++i)
    {   keycodes[i] = XCBKeySymbolsGetKeyCode(_wm.syms, keys[i].keysym);
    }
    for(i = 0; i < LENGTH(keys); ++i)
    {
        for(j = 0; keycodes[i][j] != XCB_NO_SYMBOL; ++j)
        {
            if(keys[i].keysym == XCBKeySymbolsGetKeySym(_wm.syms, keycodes[i][j], 0))
            {   
                for(k = 0; k < LENGTH(modifiers); ++k)
                {
                    XCBGrabKey(_wm.dpy, 
                            keycodes[i][j], keys[i].mod | modifiers[k], 
                            _wm.root, 1, 
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
                }
            }
        }
    }

    for(i = 0; i < LENGTH(keys); ++i)
    {   free(keycodes[i]);
    }
}

void
grid(Desktop *desk)
{
    if(!desk->clients)
    {   return;
    }
    i32 i, n, cw, ch, aw, ah, cols, rows;
    i32 nx, ny, nw, nh;
    i32 unused = 0;
    Client *c;
    Monitor *m = desk->clients->desktop->mon;
    for(n = 0, c = nexttiled(desk->clients); c; c = nexttiled(c->next))
    {   ++n;
    }

    if(!n) 
    {   return;
    }

    /* grid dimensions */
    for(rows = 0; rows <= n * 2; ++rows)
    {
        if(rows * rows >= n)
        {   break;
        }
    }
    cols = rows - !!(rows && (rows - 1) * rows >= n);
    /* window geoms (cell height/width) */
    ch = m->wh / (rows + !rows);
    cw = m->ww / (cols + !cols);
    for(i = 0, c = nexttiled(desk->clients); c; c = nexttiled(c->next))
    {
        nx = m->wx + (i / rows) * cw;
        ny = m->wy + (i % rows) * ch;
        /* adjust height/width of last row/column's windows */
        ah = !!((i + 1) % rows) * (m->wh - ch * rows);
        aw = !!(i >= rows * (cols - 1)) * (m->ww - cw * cols);

        /* _cfg.bgw without fucking everything else */
        nx += _cfg.bgw;
        ny += _cfg.bgw;

        nw = cw - (c->bw << 1) + aw;
        nh = ch - (c->bw << 1) + ah;

        nw -= _cfg.bgw;
        nh -= _cfg.bgw;

        nw -= !!aw * _cfg.bgw;
        nh -= !ah * _cfg.bgw;

        /* sanatize data */
        applysizechecks(m, &nx, &ny, &nw, &nh, &unused);
        resize(c, nx, ny, nw, nh, 0);
        ++i;
    }
}

void 
killclient(XCBWindow win, enum KillType type)
{
    /* most system pids are less than 100 */
    const uint8_t notsyspid = 100;
    if(!win)
    {   return;
    }
    if(!sendevent(win, wmatom[WMDeleteWindow]))
    {
        switch(type)
        {
            case Graceful:
                XCBKillClient(_wm.dpy, win);
                break;
            case Safedestroy:
                /* TODO */
                XCBKillClient(_wm.dpy, win);
                break;
            case Destroy:
                XCBDestroyWindow(_wm.dpy, win);
                break;
            default:
                XCBKillClient(_wm.dpy, win);
                break;
        }
    }
}

Client *
manage(XCBWindow win)
{
    Client *c, *t = NULL;
    Monitor *m = NULL;
    XCBWindow trans = 0;
    u8 transstatus = 0;
    u32 inputmask = XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_FOCUS_CHANGE|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    u8 checkbar = 0;
    XCBWindowGeometry *wg = NULL;

    /* checks */
    if(win == _wm.root)
    {   DEBUG("%s", "Cannot manage() root window.");
        return NULL;
    }
    /* barwin checks */
    if(_wm.selmon->bar)
    {
        if(_wm.selmon->bar->win == win)
        {
            DEBUG0("Cannot manage() bar window.");
            return NULL;
        }
        checkbar = !_wm.selmon->bar->win;
    }
    else
    {   checkbar = 1;
    }

    /* get cookies first */
    XCBCookie wacookie = XCBGetWindowAttributesCookie(_wm.dpy, win);
    XCBCookie wgcookie    = XCBGetWindowGeometryCookie(_wm.dpy, win);
    XCBCookie transcookie = XCBGetTransientForHintCookie(_wm.dpy, win);                        
    XCBCookie wtypecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMWindowType], 0L, UINT32_MAX, False, XCB_ATOM_ATOM);
    XCBCookie statecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMState], 0L, UINT32_MAX, False, XCB_ATOM_ATOM);
    XCBCookie sizehcookie = XCBGetWMNormalHintsCookie(_wm.dpy, win);
    XCBCookie wmhcookie   = XCBGetWMHintsCookie(_wm.dpy, win);
    XCBCookie classcookie = XCBGetWMClassCookie(_wm.dpy, win);
    XCBCookie strutpcookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMStrutPartial], 0L, 12, False, XCB_ATOM_CARDINAL);
    XCBCookie strutcookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMStrut], 0, 4, False, XCB_ATOM_CARDINAL);

    XCBGetWindowAttributes *waattributes = NULL;
    XCBWindowProperty *wtypeunused = NULL;
    XCBWindowProperty *stateunused = NULL;
    XCBSizeHints hints;
    u8 hintstatus = 0;
    XCBWMHints *wmh = NULL;
    XCBWMClass cls = { ._reply = NULL };    /* no safeguards for failure */
    XCBWindowProperty *strutpreply = NULL;
    XCBWindowProperty *strutreply = NULL;

    /* we do it here before, because we are waiting for replies and for more memory. */
    c = createclient();

    /* wait for replies */
    waattributes = XCBGetWindowAttributesReply(_wm.dpy, wacookie);
    wg = XCBGetWindowGeometryReply(_wm.dpy, wgcookie);
    transstatus = XCBGetTransientForHintReply(_wm.dpy, transcookie, &trans);
    wtypeunused = XCBGetWindowPropertyReply(_wm.dpy, wtypecookie);
    stateunused = XCBGetWindowPropertyReply(_wm.dpy, statecookie);
    hintstatus = XCBGetWMNormalHintsReply(_wm.dpy, sizehcookie, &hints);
    wmh = XCBGetWMHintsReply(_wm.dpy, wmhcookie);
    XCBGetWMClassReply(_wm.dpy, classcookie, &cls);
    strutpreply = XCBGetWindowPropertyReply(_wm.dpy, strutpcookie);
    strutreply = XCBGetWindowPropertyReply(_wm.dpy, strutcookie);

    if(!c)
    {   goto CLEANUP;
    }
    c->win = win;

    /* On Failure clear flag and ignore hints */
    hints.flags *= !!hintstatus;    

    
    /* Init struts */
    u32 *strutp = NULL; 
    u32 *strut = NULL;
    if(strutpreply)
    {   strutp = XCBGetWindowPropertyValue(strutpreply);
    }
    if(strutreply)
    {   strut = XCBGetWindowPropertyValue(strutpreply);
    }

    if(waattributes)
    {   
        if(waattributes->override_redirect)
        {   
            free(c);
            c = NULL;
            goto CLEANUP;
        }
        /* sometimes clients do dumb stuff that messes with out window managing
         * inputmask |= waattributes->your_event_mask; 
         * inputmask &= ~waattributes->do_not_propagate_mask;
         */
    }

    if(wtypeunused)
    {   
        XCBAtom *data = XCBGetPropertyValue(wtypeunused);
        updatewindowtypes(c, data, XCBGetPropertyValueLength(wtypeunused, sizeof(XCBAtom)));
    }
    if(stateunused)
    {
        XCBAtom *data = XCBGetPropertyValue(stateunused);
        updatewindowstates(c, data, XCBGetPropertyValueLength(stateunused, sizeof(XCBAtom)));
    }

    if(wg)
    {   /* init geometry */
        c->x = c->oldx = wg->x;
        c->y = c->oldy = wg->y;
        c->w = c->oldw = wg->width;
        c->h = c->oldh = wg->height;
        c->oldbw = wg->border_width;
        c->bw = wg->border_width;
    }

    if(transstatus && trans && (t = wintoclient(trans)))
    {
        c->desktop = t->desktop;
    }
    else
    {
        /* just set to current desktop */
        c->desktop = _wm.selmon->desksel;
        /* applyrules() */
    }
    m = c->desktop->mon;
    /* constrain window to monitor window area */
    if (c->x + WIDTH(c) > m->wx + m->ww)
    {   c->x = m->wx + m->ww - WIDTH(c);
    }
    if (c->y + HEIGHT(c) > m->wy + m->wh)
    {   c->y = m->wy + m->wh - HEIGHT(c);
    }
    c->x = MAX(c->x, m->wx);
    c->y = MAX(c->y, m->wy);

    /* strut partial length is 12 btw */
    u32 values[12];
    memset(values, 0, sizeof(u32) * 12);
    if(strutp)
    {   
        u32 size = XCBGetPropertyValueLength(strutpreply, sizeof(u32));
        if(size == 12)
        {   memcpy(values, strutp, 12 * sizeof(u32));
        }
    }
    else if(strut)
    {   
        u32 size = XCBGetPropertyValueLength(strutpreply, sizeof(u32));
        /* strut No partial is length of 4 */
        if(size == 4)
        {   memcpy(values, strut, 4 * sizeof(u32));
        }
    }

    i64 safevalues[12];
    u8 i;
    for(i = 0; i < 12; ++i)
    {   safevalues[i] = values[i];
    }
        
    if(checkbar)
    {
        void *wty = wtypeunused ? XCBGetPropertyValue(wtypeunused) : NULL;
        uint32_t wtylen = wty ? XCBGetPropertyValueLength(wtypeunused, sizeof(XCBAtom)) : 0;
        void *ste = stateunused ? XCBGetPropertyValue(stateunused) : NULL;
        uint32_t stelen = ste ? XCBGetPropertyValueLength(stateunused, sizeof(XCBAtom)) : 0;
        if(checknewbar(safevalues, wty, wtylen, ste, stelen, 0))
        {
            Bar *bar = managebar(_wm.selmon, win);
            if(bar)
            {
                bar->x = c->x;
                bar->y = c->y;
                bar->w = c->w;
                bar->h = c->h;
                setshowbar(bar, 1);
                updatebargeom(_wm.selmon);
                updatebarpos(_wm.selmon);
                XCBSelectInput(_wm.dpy, win, inputmask);
                XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&win, 1);
                /* map the window or we get errors */
                XCBMapWindow(_wm.dpy, win);
            }
            free(c);
            c = NULL;
            goto CLEANUP;
        }
        else
        {   DEBUG("Not a bar: [%d]", win);
        }
    }

    /* Custom stuff */
    setborderwidth(c, _cfg.bw);
    setbordercolor32(c, _cfg.bcol);
    XCBSetWindowBorderWidth(_wm.dpy, win, c->bw);
    XCBSetWindowBorder(_wm.dpy, win, c->bcol);
    configure(c);   /* propagates border_width, if size doesn't change */
    updatetitle(c);
    updatesizehints(c, &hints);
    updatewmhints(c, wmh);
    XCBSelectInput(_wm.dpy, win, inputmask);
    grabbuttons(win, 0);

    attach(c);
    attachstack(c);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&win, 1);
    setclientstate(c, XCB_WINDOW_NORMAL_STATE);
    /* map the window or we get errors */
    XCBMapWindow(_wm.dpy, win);

    if(c->desktop == _wm.selmon->desksel)
    {   unfocus(_wm.selmon->desksel->sel, 0);
    }
    /* inherit previous client state */
    if(c->desktop && c->desktop->sel)
    {   setfullscreen(c, ISFULLSCREEN(c->desktop->sel) || ISFULLSCREEN(c));
    }
    c->desktop->sel = c;
    arrange(c->desktop);
    /* client could be floating so we pass NULL for focus */
    focus(NULL);
    goto CLEANUP;
CLEANUP:
    /* reply cleanup */
    free(waattributes);
    free(wmh);
    free(stateunused);
    free(wtypeunused);
    free(wg);
    XCBWipeGetWMClass(&cls);
    free(strutpreply);
    free(strutreply);

    return c;
}

Bar *
managebar(Monitor *m, XCBWindow win)
{
    DEBUG("New bar: [%d]", win);
    if(!m->bar)
    {   
        m->bar = createbar();
        if(!m->bar)
        {   return NULL;
        }
    }
    if(!win)
    {   return NULL;
    }
    m->bar->win = win;
    return m->bar;
}

void
maximize(Client *c)
{
    const Monitor *m = c->desktop->mon;
    resize(c, m->wx, m->wy, m->ww, m->wh, 0);
}

void
maximizehorz(Client *c)
{
    const Monitor *m = c->desktop->mon;
    
    resize(c, m->wx, c->y, m->ww, c->h, 0);
}

void
maximizevert(Client *c)
{
    const Monitor *m = c->desktop->mon;

    resize(c, c->x, m->wy, c->w, m->wh, 0);
}


void
monocle(Desktop *desk)
{
    if(!desk->clients)
    {   return;
    }
    Client *c;
    Monitor *m = desk->clients->desktop->mon;
    i32 nw, nh;
    i32 nx = m->wx;
    i32 ny = m->wy;
    i32 unused = 0;

    for(c = nexttiled(desk->clients); c; c = nexttiled(c->next))
    {
        nw = m->ww - (c->bw * 2);
        nh = m->wh - (c->bw * 2);
        applysizechecks(m, &nx, &ny, &nw, &nh, &unused);
        resize(c, nx, ny, nw, nh, 0); 
    }
}

Client *
nextclient(Client *c)
{
    return c ? c->next : c;
}

Desktop *
nextdesktop(Desktop *desk)
{
    return desk ? desk->next : desk;
}

Monitor *
nextmonitor(Monitor *m)
{
    return m ? m->next : m;
}

Client *
nextstack(Client *c)
{
    return c ? c->snext : c;
}

Client *
nexttiled(Client *c)
{
    for(; c && (!ISVISIBLE(c) || ISFLOATING(c)); c = nextclient(c));
    return c;
}

Client *
nextvisible(Client *c)
{
    for(; c && !ISVISIBLE(c); c = c->next);
    return c;
}

Client *
lastvisible(Client *c)
{
    for(; c && !ISVISIBLE(c); c = c->prev);
    return c;
}

void
quit(void)
{
    _wm.running = 0;
    wakeupconnection();
}

Monitor *
recttomon(i16 x, i16 y, u16 w, u16 h)
{
	Monitor *m, *r = _wm.selmon;
	i32 a, area = 0;

	for (m = _wm.mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

/* why arent these their "correct" format of i16's well if we did do that the digits would be a bit busy being overflown/underflown wouldnt they?
 * Defeating the entire purpose of bound checking.
 */
void
resize(Client *c, i32 x, i32 y, i32 width, i32 height, uint8_t interact)
{
    if(applysizehints(c, &x, &y, &width, &height, interact))
    {   resizeclient(c, x, y, width, height);
    }
}

void 
resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    c->oldx = c->x;
    c->oldy = c->y;
    c->oldw = c->w;
    c->oldh = c->h;
    /* due to the suprisingly expensive nature of configuring clients we can save some cycles here */
    if(x != c->x || y != c->y)
    {   XCBMoveWindow(_wm.dpy, c->win, x, y);
    }
    if(width != c->w || height != c->h)
    {   XCBResizeWindow(_wm.dpy, c->win, width, height);
    }
    c->x = x;
    c->y = y;
    c->w = width;
    c->h = height;
    XCBSetWindowBorderWidth(_wm.dpy, c->win, c->bw);
    configure(c);
}

void
restack(Desktop *desk)
{
    Client *c;
    XCBWindowChanges wc;

    uint16_t cc = 0;    /* client counter */
    for(c = desk->stack; c; c = nextstack(c), ++cc);
    /* no clients */
    if(!cc)
    {   return;
    }

    wc.stack_mode = XCB_STACK_MODE_BELOW;
    if(desk->stack->desktop->mon->bar && SHOWBAR(desk->stack->desktop->mon->bar))
    {   wc.sibling = desk->stack->desktop->mon->bar->win;
    }
    else
    {   wc.sibling = _wm.root;
    }

    /* configure windows */
    for(; c; c = nextstack(c))
    {
        XCBConfigureWindow(_wm.dpy, c->win, XCB_CONFIG_WINDOW_SIBLING|XCB_CONFIG_WINDOW_STACK_MODE, &wc);
        wc.sibling = c->win;
    }

    /* TODO use a sorting algorithm maybe, and figure out some faster way without possible buffer overruns */

    /* this enables win10 floating if we pick the floating layout */
    if(layouts[desk->layout].arrange != floating)
    {
        for(c = desk->stack; c; c = nextstack(c))
        {
            if(ISFLOATING(c))
            {   XCBRaiseWindow(_wm.dpy, c->win);
            }
        }
    }

    for(c = desk->stack; c; c = nextstack(c))
    {
        if(ISALWAYSONTOP(c))
        {   
            if(ISFLOATING(c))
            {
                XCBRaiseWindow(_wm.dpy, c->win);
            }
            else
            {   DEBUG0("Client should be floating, but isnt, this shouldnt be possible; CHECK: ALWAYSONTOP ");
            }
        }
    }

    for(c = desk->stack; c; c = nextstack(c))
    {
        if(ISDIALOG(c))
        {   XCBRaiseWindow(_wm.dpy, c->win);
        }
    }

    for(c = desk->stack; c; c = nextstack(c))
    {
        if(ISMODAL(c)) 
        {   XCBRaiseWindow(_wm.dpy, c->win);
        }
    }
}

void
restart(void)
{
    _wm.restart = 1;
    quit();
}

void 
run(void)
{
    XCBGenericEvent *ev = NULL;
    XCBSync(_wm.dpy);
    while(_wm.running && (((ev = XCBPollForEvent(_wm.dpy))) || (XCBNextEvent(_wm.dpy, &ev))))
    {
        eventhandler(ev);
        free(ev);
        ev = NULL;
    }
    _wm.has_error = XCBCheckDisplayError(_wm.dpy);
}

/* scan for clients initally */
void
scan(void)
{
    u16 i, num;
    XCBWindow *wins = NULL;
    const XCBCookie cookie = XCBQueryTreeCookie(_wm.dpy, _wm.root);
    XCBQueryTree *tree = NULL;

    if((tree = XCBQueryTreeReply(_wm.dpy, cookie)))
    {
        num = tree->children_len;
        wins = XCBQueryTreeChildren(tree);
        if(wins)
        {
            XCBCookie wa[num];
            XCBCookie wastates[num];
            XCBCookie tfh[num];
            for(i = 0; i < num; ++i)
            {   
                wa[i] = XCBGetWindowAttributesCookie(_wm.dpy, wins[i]);
                /* this specifically queries for the state which wa[i] might fail to provide */
                wastates[i] = XCBGetWindowPropertyCookie(_wm.dpy, wins[i], wmatom[WMState], 0L, 2L, False, wmatom[WMState]);
                tfh[i] = XCBGetTransientForHintCookie(_wm.dpy, wins[i]);
            }
            
            XCBGetWindowAttributes *replies[num];
            XCBGetWindowAttributes *replystates[num];
            /* filled data no need to free */
            XCBWindow trans[num];
            uint8_t hastrans = 0;
            /* get them replies back */
            for(i = 0; i < num; ++i)
            {
                replies[i] = XCBGetWindowAttributesReply(_wm.dpy, wa[i]);
                replystates[i] = XCBGetWindowAttributesReply(_wm.dpy, wastates[i]);
                hastrans = XCBGetTransientForHintReply(_wm.dpy, tfh[i], &trans[i]);

                if(!hastrans)
                {   trans[i] = 0;
                }
                /* override_redirect only needed to be handled for old windows */
                /* X auto redirects when running wm so no need to do anything else */
                if(replies[i]->override_redirect || trans[i]) 
                {   continue;
                }
                if(replies[i] && replies[i]->map_state == XCB_MAP_STATE_VIEWABLE)
                {   manage(wins[i]);
                }
                else if(replystates[i] && replystates[i]->map_state == XCB_WINDOW_ICONIC_STATE)
                {   manage(wins[i]);
                }
            }

            /* now the transients */
            for(i = 0; i <  num; ++i)
            {   
                if(trans[i])
                {
                    if(replies[i]->map_state == XCB_MAP_STATE_VIEWABLE && replystates[i] && replystates[i]->map_state == XCB_WINDOW_ICONIC_STATE)
                    {
                        /* technically we shouldnt have to do this but just in case */
                        if(!wintoclient(wins[i]))
                        {   manage(wins[i]);
                        }
                    }
                }
                free(replies[i]);
                free(replystates[i]);
            }
        }
        free(tree);
    }
    else
    {   DEBUG("%s", "Failed to scan for clients.");
    }
}

uint8_t
sendevent(XCBWindow win, XCBAtom proto)
{
    XCBWMProtocols protocols = { ._reply = NULL };
    uint8_t exists = 0;

    XCBCookie wmprotocookie = XCBGetWMProtocolsCookie(_wm.dpy, win, wmatom[WMProtocols]);

    if(XCBGetWMProtocolsReply(_wm.dpy, wmprotocookie, &protocols))
    {
        uint32_t i = protocols.atoms_len;
        while(!exists && i--)
        {   exists = protocols.atoms[i] == proto;
        }
        if(!exists)
        {   DEBUG("Could not find sendevent request for [%d]", win);
        }
    }
    else
    {   DEBUG("Not WMProtocols for window [%d]", win);
    }

    if(exists)
    {
        XCBClientMessageEvent ev;
        ev.type = wmatom[WMProtocols];
        ev.response_type = XCB_CLIENT_MESSAGE;
        ev.window = win;
        ev.format = 32;
        ev.data.data32[0] = proto;
        ev.data.data32[1] = XCB_CURRENT_TIME;
        XCBSendEvent(_wm.dpy, win, False, XCB_NONE, (const char *)&ev);
    }

    XCBWipeGetWMProtocols(&protocols);
    return exists;
}

void
sendmon(Client *c, Monitor *m)
{
    if(!c->desktop)
    {   c->desktop = m->desksel;
    }
    if(c->desktop->mon == m)
    {   DEBUG0("Cant send client to itself.");
        return;
    }
    unfocus(c, 1);
    detachcompletely(c);
    c->desktop = m->desksel;
    setclientdesktop(c, m->desksel);
    attach(c);
    attachstack(c);
    focus(NULL);
    /* arrangeall() */
}

void
setalwaysontop(Client *c, u8 state)
{
    SETFLAG(c->wstateflags, _STATE_ABOVE, !!state);
}

void
setborderalpha(Client *c, uint8_t alpha)
{
    /* TODO */
}

void
setbordercolor(Client *c, uint8_t red, uint8_t green, uint8_t blue)
{
    c->bcol = blue + (green << 8) + (red << 16);
}

void
setbordercolor32(Client *c, uint32_t col)
{   
    c->bcol = col;
}

void
setborderwidth(Client *c, uint16_t border_width)
{
    c->oldbw = c->bw;
    c->bw = border_width;
}

void
setclientdesktop(Client *c, Desktop *desk)
{
    detachcompletely(c);
    c->desktop = desk;
    attach(c);
}

void
setclientstate(Client *c, u8 state)
{
    i32 data[2] = { state, XCB_NONE };
    
    XCBChangeProperty(_wm.dpy, c->win, wmatom[WMState], wmatom[WMState],
            32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}

void
setdesktoplayout(Desktop *desk, uint8_t layout)
{
    desk->olayout = layout;
    desk->layout = layout;
}

void
setwtypedesktop(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DESKTOP, !!state);
}

void
setwtypedock(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DOCK, !!state);
}

void
setwtypetoolbar(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_TOOLBAR, !!state);
}

void
setwtypemenu(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_MENU, !!state);
}

void
setwtypeutility(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_UTILITY, !!state);
}

void
setwtypesplash(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_SPLASH, !!state);
}

void
setwtypedialog(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DIALOG, !!state);
}

void
setwtypedropdownmenu(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DROPDOWN_MENU, !!state);
}

void
setwtypepopupmenu(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_POPUP_MENU, !!state);
}

void
setwtypetooltip(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_TOOLTIP, !!state);
}

void
setwtypenotification(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_NOTIFICATION, !!state);
}

void
setwtypecombo(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_COMBO, !!state);
}

void
setwtypednd(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DND, !!state);
}

void
setwtypenormal(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_NORMAL, !!state);
}

void
setskippager(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SKIP_PAGER, !!state);
}

void
setskiptaskbar(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SKIP_TASKBAR, !!state);
}

void
setalwaysonbottom(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_BELOW, !!state);
}

void
setfullscreen(Client *c, u8 state)
{
    const Monitor *m = c->desktop->mon;
    if(state && !ISFULLSCREEN(c))
    {
        XCBChangeProperty(_wm.dpy, c->win, netatom[NetWMState], XCB_ATOM_ATOM, 32,
        XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom[NetWMStateFullscreen], 1);
        setborderwidth(c, c->bw);
        setborderwidth(c, 0);
        resizeclient(c, m->mx, m->wy, m->mw, m->mh);
        XCBRaiseWindow(_wm.dpy, c->win);
    }
    else if(!state && ISFULLSCREEN(c))
    {
        XCBChangeProperty(_wm.dpy, c->win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
        XCB_PROP_MODE_REPLACE, (unsigned char *)0, 0);
        setborderwidth(c, c->oldbw);
        resizeclient(c, c->oldx, c->oldy, c->oldw, c->oldh);
    }
    SETFLAG(c->wstateflags, _STATE_FULLSCREEN, !!state);
}

void
setfocus(Client *c)
{
    if(!NEVERFOCUS(c))
    {
        XCBSetInputFocus(_wm.dpy, c->win, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&(c->win), 1);
        SETFLAG(c->wstateflags, _STATE_FOCUSED, 1);
    }
    sendevent(c->win, wmatom[WMTakeFocus]);
}

void 
sethidden(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_HIDDEN, !!state);
}

void
setmaximizedvert(Client *c, uint8_t state)
{
    const Monitor *m = c->desktop->mon;
    SETFLAG(c->wstateflags, _STATE_MAXIMIZED_VERT, !!state);
    if(state)
    {
        if(!ISMAXVERT(c))
        {
            resize(c, c->x, c->y, c->w, m->wh, 0);
        }
    }
    else
    {
        if(ISMAXVERT(c))
        {                                                          /* fallback */
            resize(c, c->x, c->y, c->w, c->oldh != m->wh ? c->oldh : c->oldh / 2, 0);
        }
    }
}

void
setmaximizedhorz(Client *c, uint8_t state)
{
    const Monitor *m = c->desktop->mon;
    SETFLAG(c->wstateflags, _STATE_MAXIMIZED_HORZ, !!state);
    if(state)
    {
        if(!ISMAXHORZ(c))
        {
            resize(c, c->x, c->y, m->ww, c->h, 0);
        }
    }
    else
    {
        if(ISMAXHORZ(c))
        {                                                     /* Fallback. */
            resize(c, c->x, c->y, c->oldw != m->ww ? c->oldw : c->oldw / 2, c->h, 0);
        }
    }
}

void
setshaded(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SHADED, !!state);
}

/* TODO: HERE */

void
setmodal(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_MODAL, !!state);
}

void
setneverfocus(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_NEVERFOCUS, !!state);
}

void
setsticky(Client *c, u8 state)
{
    const XCBWindow win = c->win;
    const XCBAtom replace = !!state * netatom[NetWMStateSticky];
    XCBChangeProperty(_wm.dpy, win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
            XCB_PROP_MODE_REPLACE, (unsigned char *)&replace, !!replace);

    SETFLAG(c->wstateflags, _STATE_STICKY, !!state);
}

void
setup(void)
{
    /* clean up any zombies immediately */
    sighandler();

    /* startup wm */
    _wm.running = 1;
    _wm.syms = XCBKeySymbolsAlloc(_wm.dpy);
    _wm.sw = XCBDisplayWidth(_wm.dpy, _wm.screen);
    _wm.sh = XCBDisplayHeight(_wm.dpy, _wm.screen);
    _wm.root = XCBRootWindow(_wm.dpy, _wm.screen);
    _wm.ct = ThreadGetSelf();

    if(!_wm.syms)
    {   
        cleanup();
        DIECAT("%s", "Could not establish connection with keyboard (OutOfMemory)");
    }

    if(!_wm.ct)
    {
        cleanup();
        DIECAT("%s", "Could not get the current thread.");
    }

    /* TODO testing default settings */
    _cfg.nmaster = 1;
    _cfg.hoverfocus = 0;

    _cfg.bw = 1;
    _cfg.bgw = 0;

    _cfg.bcol = 100 + (255 << 8) + (123 << 16) + (65 << 24);

    _cfg.snap = 10;
    _cfg.rfrate = 120;

    _cfg.bh = 10;
    _cfg.maxcc = 256;

    _cfg.mfact = 0.55f;
    /* Xorg Default is 256 or 255 dont remember. */

    /* Most java apps require this see:
     * https://wiki.archlinux.org/title/Java#Impersonate_another_window_manager
     * https://wiki.archlinux.org/title/Java#Gray_window,_applications_not_resizing_with_WM,_menus_immediately_closing
     * for more information.
     * "Hard coded" window managers to ignore "Write Once, Debug Everywhere"
     * Not sure why it just doesnt default to that if it cant detect a supported wm.
     * This fixes java apps just having a blank white screen on some screen instances.
     * One example is Ghidra, made by the CIA.
     */
    /* TODO: Maybe just change the name quickly when a app lanches and change it back when done? probably in createnotify event? */
    _cfg.wmname = "LG3D";

    updategeom();
    const XCBCookie utf8cookie = XCBInternAtomCookie(_wm.dpy, "UTF8_STRING", False);
    XCBInitAtoms(_wm.dpy, wmatom, netatom);
    const XCBAtom utf8str = XCBInternAtomReply(_wm.dpy, utf8cookie);
    /* supporting window for NetWMCheck */
    _wm.wmcheckwin = XCBCreateSimpleWindow(_wm.dpy, _wm.root, 0, 0, 1, 1, 0, 0, 0);
    XCBSelectInput(_wm.dpy, _wm.wmcheckwin, XCB_NONE);
    XCBChangeProperty(_wm.dpy, _wm.wmcheckwin, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm.wmcheckwin, 1);
    XCBChangeProperty(_wm.dpy, _wm.wmcheckwin, netatom[NetWMName], utf8str, 8, XCB_PROP_MODE_REPLACE, _cfg.wmname, strlen(_cfg.wmname) + 1);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm.wmcheckwin, 1);
    /* EWMH support per view */
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom, NetLast);
    XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetClientList]);
    
    updatedesktopnum();
    updatedesktop();
    updatedesktopnames();
    updateviewport();

    XCBWindowAttributes wa;
    /* xcb_event_mask_t */
    /* ~0 causes event errors because some event masks override others, for some reason... */
    wa.event_mask = 
                    XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE
                    |XCB_EVENT_MASK_POINTER_MOTION
                    |XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_LEAVE_WINDOW
                    |XCB_EVENT_MASK_STRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_PROPERTY_CHANGE
                    ;   /* the ; is here just so its out of the way */
    XCBChangeWindowAttributes(_wm.dpy, _wm.root, XCB_CW_EVENT_MASK, &wa);
    XCBSelectInput(_wm.dpy, _wm.root, wa.event_mask);
    /* init numlock */
    updatenumlockmask();
    grabkeys();
    focus(NULL);
}

void
seturgent(Client *c, uint8_t state) 
{
    XCBCookie wmhcookie = XCBGetWMHintsCookie(_wm.dpy, c->win);
    XCBWMHints *wmh = NULL;
    SETFLAG(c->wstateflags, _STATE_DEMANDS_ATTENTION, !!state);
    if(state)
    {   /* set window border */   
    }
    else
    {   /* set window border */   
    }

    if((wmh = XCBGetWMHintsReply(_wm.dpy, wmhcookie)))
    {
        wmh->flags = state ? (wmh->flags | XCB_WM_HINT_URGENCY) : (wmh->flags & ~XCB_WM_HINT_URGENCY);
        XCBSetWMHintsCookie(_wm.dpy, c->win, wmh);
        free(wmh);
    }
    /* drawbar */
}

void
updatedesktop(void)
{
    i32 data[1] = { _wm.selmon->desksel->num };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetCurrentDesktop], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 1);
}

void
updatedesktopnames(void)
{
    char names[_wm.selmon->deskcount];
    u16 i;
    for(i = 0; i < _wm.selmon->deskcount; ++i)
    {   names[i] = i;
    }
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopNames], XCB_ATOM_STRING, 8, XCB_PROP_MODE_REPLACE, names, _wm.selmon->deskcount);
}

void
updatedesktopnum(void)
{
    i32 data[1] = { _wm.selmon->deskcount };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetNumberOfDesktops], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 1);
}

void
updateviewport(void)
{
    i32 data[2] = { 0, 0 };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopViewport], 
            XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}


void
showhide(Client *c)
{
    const Monitor *m = c->desktop->mon;
    if(ISVISIBLE(c))
    {   XCBMoveWindow(_wm.dpy, c->win, c->x, c->y);
    }
    else
    {   
        const i16 x = (m->mx - (WIDTH(c) / 2));
        XCBMoveWindow(_wm.dpy, c->win, x, c->y);
    }
}

void
sigchld(int signo) /* signal */
{
    (void)signo;

    struct sigaction sa;
    /* donot transform children into zombies when they terminate */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);

    /* wait for childs (zombie proccess) to die */
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void
sighandler(void)
{
    /* sig info: https://faculty.cs.niu.edu/~hutchins/csci480/signals.htm 
     * */
    if(signal(SIGCHLD, &sigchld) == SIG_ERR)
    {   DIECAT("%s", "FATAL: CANNOT_INSTALL_SIGCHLD_HANDLER");
    }
    /* wait for zombies to die */
    sigchld(0);
    if(signal(SIGTERM, &sigterm) == SIG_ERR) 
    {   
        DIECAT("%s", "FATAL: CANNOT_INSTALL_SIGTERM_HANDLER");
        signal(SIGTERM, SIG_DFL); /* default signal */
    }

    if(signal(SIGHUP, &sighup) == SIG_ERR) 
    {   
        DEBUG("%s", "WARNING: CANNOT_INSTALL_SIGHUP_HANDLER");
        signal(SIGHUP, SIG_DFL); /* default signal */
    }
    if(signal(SIGINT, &sigterm) == SIG_ERR)
    {   
        DEBUG("%s", "WARNING: CANNOT_INSTALL_SIGINT_HANDLER");
        signal(SIGINT, SIG_DFL);
    }
}

void
sighup(int signo) /* signal */
{
    restart();
}

void
sigterm(int signo)
{
    quit();
}

void
specialconds(int argc, char *argv[])
{
    /* local support */
    char *err = strerror_l(errno, uselocale((locale_t)0));
    if(err)
    {   DEBUG("%s", strerror_l(errno, uselocale((locale_t)0)));
    }

    err = NULL;
    switch(_wm.has_error)
    {
        case XCB_CONN_ERROR:
            err =   "Could not connect to the XServer for whatever reason BadConnection Error.";
            break;
        case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
            err =   "The XServer could not find an extention ExtensionNotSupported Error.\n"
                    "This is more or less a developer error, but if you messed up your build this could happen."
                    ;
            break;
        case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
            err =   "The XServer died due to an OutOfMemory Error.\n"
                    "This can be for several reasons but the the main few are as follows:\n"
                    "1.) Alloc Failure, due to system calls failing xcb could die, but probably didnt.\n"
                    "2.) No Memory, basically you ran out of memory for the system.\n"
                    "3.) ulimit issues, basically your system set memory limits programs."
                    ;
            break;
        case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
            err =   "The XServer died due to an TooBigRequests Error.\n"
                    "Basically we either.\n"
                    "A.) Dont use the BigRequests extension and we ran into a \"Big Request.\" \n"
                    "Or\n"
                    "B.) We did use the BigRequests extension and some rogue app sent a massive requests bigger than ~16GiB of data.\n"
                    "This mostly occurs with rogue app's for B but more likely for A due to regular requests being small ~256kB."
                    ;
            break;
        case XCB_CONN_CLOSED_PARSE_ERR:
            err =   "The XServer died to due an BadParse Error.\n"
                    "While the XServer probably didnt and shouldnt die on a BadParse error it really just depends.\n"
                    "In essence however the XServer simply couldnt parse som form of requests that was sent."
                    ;
            break;
        case XCB_CONN_CLOSED_INVALID_SCREEN:
            err =   "Could not connect to specified screen.\n"
                    "You should check if your DISPLAY variable is correctly set, or if you incorrectly passed a screen as a display connection.\n"
                    "You may have incorrectly set your DISPLAY variable using setenv(\"DISPLAY\", (char *)) the correct format is as follows:\n"
                    "char *display = \":0\"; /* you pass in the thing ':' and the the number so display 5 would be \":5\" */\n"
                    "setenv(\"DISPLAY\", display);\n"
                    "The same applies when connection to the XServer using so XOpenDisplay(\":2\"); would open the second display.\n"
                    "For more information see: https://www.x.org/releases/X11R7.7/doc/man/man3/XOpenDisplay.3.xhtml"
                    ;
            break;
        case 0:
            /* no error occured */
            break;
        default:
            err = "The XServer died with an unexpected error.";
            break;
    }

    if(err)
    {   DEBUG("%s\nError code: %d", err, _wm.has_error);
    }


    /* this is the end of the exithandler so we dont really care if we segfault here if at all.
     * But this covers some cases where system skips to here. (AKA manual interrupt)
     * Really this is mostly just to prevent XKeyboard saying we didnt free stuff. (its annoying)
     * Though it sometimes doesnt work, it works 90% of the time which is good enough.
     */
    if(_wm.dpy)
    {   cleanup();
    }

    if(_wm.restart)
    {   execvp(argv[0], argv);
        /* UNREACHABLE */
        DEBUG("%s", "Failed to restart " NAME);
    }
}

void
startup(void)
{
    if(!setlocale(LC_CTYPE, ""))
    {   fputs("WARN: NO_LOCALE_SUPPORT\n", stderr);
    }
    const char *display = NULL;
    _wm.dpy = XCBOpenDisplay(display, &_wm.screen);
    DEBUG("DISPLAY -> %s", display ? display : getenv("DISPLAY"));

    if(!_wm.dpy)
    {   DIECAT("%s", "FATAL: Cannot Connect to X Server.");
    }
    checkotherwm();
    XCBSetErrorHandler(xerror);

    /* This allows for execvp and exec to only spawn process on the specified display rather than the default varaibles */
    if(display)
    {   setenv("DISPLAY", display, 1);
    }
    if(!ToggleInit())
    {   DIECAT("%s", "FATAL: Cannot create another thread for toggle function.");
    }
    atexit(exithandler);
}

void
tile(Desktop *desk)
{
    i32 h = 0, mw = 0, my = 0, ty = 0;
    i32 n = 0, i = 0;
    i32 nx = 0, ny = 0;
    i32 nw = 0, nh = 0;
    i32 unused = 0;
    Client *c = NULL;
    Monitor *m = NULL;

    if(!desk->clients)
    {   return;
    }

    m = desk->clients->desktop->mon;
    for(n = 0, c = nexttiled(desk->clients); c; c = nexttiled(c->next))
    {   ++n;
    }

    if(!n) 
    {   return;
    }
    
    if(n > _cfg.nmaster)
    {   mw = _cfg.nmaster ? m->ww * _cfg.mfact: 0;
    }
    else
    {   mw = m->ww;
    }

    for (i = my = ty = 0, c = nexttiled(desk->clients); c; c = nexttiled(c->next), ++i)
    {
        if (i < _cfg.nmaster)
        {
            h = (m->wh - my) / (MIN(n, _cfg.nmaster) - i);
            nx = m->wx;
            ny = m->wy + my;
            nw = mw - c->bw * 2;
            nh = h - c->bw * 2;

            /* we divide nw also to get even gaps
             * if we didnt the center gap would be twices as big
             * Although this may be desired, one would simply remove the shift ">>" by 1 in nw 
             */
            nx += _cfg.bgw;
            ny += _cfg.bgw;
            nw -= _cfg.bgw * 2;
            nh -= _cfg.bgw * 2;
            applysizechecks(m, &nx, &ny, &nw, &nh, &unused);
            resize(c, nx, ny, nw, nh, 0);
                                                                        /* spacing for windows below */
            if (my + HEIGHT(c) < (unsigned int)m->wh) my += HEIGHT(c) + _cfg.bgw;
        }
        else
        {
            h = (m->wh - ty) / (n - i);
            nx = m->wx + mw;
            ny = m->wy + ty;
            nw = m->ww - mw - (c->bw << 1);
            nh = h - c->bw * 2;

            nx += _cfg.bgw / 2;
            ny += _cfg.bgw;
            nw -= _cfg.bgw * 2;
            nh -= _cfg.bgw * 2;
            applysizechecks(m, &nx, &ny, &nw, &nh, &unused);
            resize(c, nx, ny, nw, nh, 0);
                                                                    /* spacing for windows below */ 
            if (ty + HEIGHT(c) < (unsigned int)m->wh) ty += HEIGHT(c) + _cfg.bgw;
        }
    }
}

void
unfocus(Client *c, uint8_t setfocus)
{
    if(!c)   
    {   return;
    }
    grabbuttons(c->win, 0);
    c->desktop->lastfocused = c;
    XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);
    if(setfocus)
    {   
        XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
    }
    SETFLAG(c->wstateflags, _STATE_FOCUSED, 0);
}

#ifdef XINERAMA
static int
isuniquegeom(XCBXineramaScreenInfo *unique, size_t n, XCBXineramaScreenInfo *info)
{
    while(n--)
    {   if(unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width && unique[n].height == info->height)
        {   return 0;
        }
    }
    return 1;
}
#endif
int
updategeom(void)
{
	int dirty = 0;

#ifdef XINERAMA
    int xienabled = 0;
    int xiactive = 0;
    XCBQueryExtension *extrep = NULL;
    XCBXineramaIsActive *xia = NULL;

    /* check if we even have the extension enabled */
    extrep = (XCBQueryExtension *)xcb_get_extension_data(_wm.dpy, &xcb_xinerama_id);
    xienabled = (extrep && !extrep->present);

    if(xienabled)
    {
        xcb_xinerama_is_active_cookie_t xcookie = xcb_xinerama_is_active(_wm.dpy);
        /* let event handler handle a Xinerama error */
        xia = xcb_xinerama_is_active_reply(_wm.dpy, xcookie, NULL);
        xiactive = xia && xia->state;
    }
    /* assume no error and proceed */
    if(xiactive)
    {
        int i, j, n, nn;
        Client *c = NULL;
        Monitor *m = NULL;
        XCBGenericError *err = NULL;
        XCBXineramaQueryScreens *xsq = NULL;
        XCBXineramaScreenInfo *info = NULL, *unique = NULL;

        xsq = xcb_xinerama_query_screens_reply(_wm.dpy, xcb_xinerama_query_screens_unchecked(_wm.dpy), &err);
        if(!xsq || err)
        {
            /* were fucked */
            DIECAT("%s", "Xinerama is broken, contact a developer to fix this issue");
        }


        info = xcb_xinerama_query_screens_screen_info(xsq);
        nn = xcb_xinerama_query_screens_screen_info_length(xsq);


        for(n = 0, m = _wm.mons; m; m = m->next, ++n);
		/* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(xcb_xinerama_query_screens_reply_t));
        for(i = 0, j = 0; i < nn; ++i)
        {   if(isuniquegeom(unique, j, &info[i]))
            {   memcpy(&unique[j++], &info[i], sizeof(xcb_xinerama_screen_info_t));
            }
        }

        free(xsq);
        nn = j;

		/* new monitors if nn > n */
		for (i = n; i < nn; ++i)
        {
			for (m = _wm.mons; m && m->next; m = m->next);
			if (m)
            {   m->next = createmon();
            }
			else
            {   _wm.mons = createmon();
            }
		}
		for (i = 0, m = _wm.mons; i < nn && m; m = m->next, ++i)
			if (i >= n
			|| unique[i].x_org != m->mx || unique[i].y_org != m->my
			|| unique[i].width != m->mw || unique[i].height != m->mh)
			{
				dirty = 1;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
                /* we should update the bar position if we have one */
                updatebarpos(m);
			}
		/* removed monitors if n > nn */
		for (i = nn; i < n; ++i)
        {
			for (m = _wm.mons; m && m->next; m = m->next);
			while ((c = m->desktops->clients)) 
            {
				dirty = 1;
				m->desktops->clients = c->next;
				detachstack(c);
                c->desktop = _wm.mons->desktops;
                /* TODO desktops might break. */
				attach(c);
				attachstack(c);
			}
			if (m == _wm.selmon)
				_wm.selmon = _wm.mons;
			cleanupmon(m);
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{  /* default monitor setup */
		if (!_wm.mons)
        {   _wm.mons = createmon();
        }
		if (_wm.mons->mw != _wm.sw || _wm.mons->mh != _wm.sh) 
        {
			dirty = 1;
			_wm.mons->mw = _wm.mons->ww = _wm.sw;
			_wm.mons->mh = _wm.mons->wh = _wm.sh;
            /* we should update the bar position if we have one */
            updatebarpos(_wm.mons);
		}
	}
	if (dirty) 
    {
		_wm.selmon = _wm.mons;
		_wm.selmon = wintomon(_wm.root);
	}
	return dirty;
}

void
unmanage(Client *c, uint8_t destroyed)
{
    Desktop *desk = c->desktop;
    if(!c)
    {   return;
    }
    if(c->desktop->lastfocused == c)
    {   c->desktop->lastfocused = NULL;
    }
    if(!destroyed)
    {   
        /* TODO causes alot of errors for some reason even if its not "destroyed" */
    }
    /* TODO
     * Memory leak if a client is unmaped and maped again
     * (cause we would get the same input focus twice)
     */
    detachcompletely(c);
    cleanupclient(c);
    focus(NULL);
    updateclientlist();
    arrange(desk);
    c = NULL;
}

void
unmanagebar(Bar *bar)
{
    if(bar)
    {   memset(bar, 0, sizeof(Bar));
    }
}

void
updatebarpos(Monitor *m)
{
    if(!m->bar)
    {   return;
    }
    Bar *bar = m->bar;
    if(SHOWBAR(m->bar))
    {
        /* side bar checks */
        if(bar->w <= m->mw / 2 || bar->h >= m->mh / 2)
        {
            m->wx = m->mx;
            m->ww = m->mw;
            m->ww -= bar->w;
            /* if left */
            if(bar->x + bar->w / 2 <= m->mx + m->mw / 2)
            {
                bar->x = m->wx;
                m->wx += bar->w;
            }
            /* else right */
            else
            {
                bar->x = m->wx + m->ww;
            }
            return;
        }
        /* top/bottom bar */
        m->wy = m->my;
        m->wh = m->mh;
        m->wh -= m->bar->h;
        /* is it topbar? */
        if(bar->y + bar->h / 2 >= m->my + m->mh / 2)
        {
            bar->y = m->wy;
            m->wy += bar->h;
        }
        else
        {   
            bar->y = m->wy + m->wh;
        }
    }
    else
    {   
        /* side bar checks */
        if(bar->w <= m->mw / 2 || bar->h >= m->mh / 2)
        {
            m->wx = m->mx;
            m->ww = m->mw;
            /* mostly for compositors animating it sliding out to the side */
            /* if left */
            if(bar->x + bar->w / 2 <= m->mx + m->mw / 2)
            {
                bar->x = -bar->w;
            }
            /* else right */
            else
            {
                bar->x += bar->w;
            }
            return;
        }
        /* top/bottom bar */
        m->wy = m->my;
        m->wh = m->mh;
        bar->y = -bar->h;
    }
}

void
updatebargeom(Monitor *m)
{
}

void
updateclientlist(void)
{
    Client *c;
    Monitor *m;
    Desktop *desk;
    
    XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetClientList]);


    for(m = _wm.mons; m; m = nextmonitor(m))
    {
        for(desk = m->desktops; desk; desk = nextdesktop(desk))
        {
            for(c = desk->clients; c; c = nextclient(c))
            {   XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&(c->win), 1);
            }
        }
    }
}



/* TODO xcb_key_symbols_get_keycode -> xcb_key_symbols_get_keysym is called a fuck ton and is slow as hell like 20% of manage() slow (callgrind) */
void
updatenumlockmask(void)
{
    /* taken from i3 */
    XCBKeyboardModifier *reply;
    XCBGenericError *err;

    reply = xcb_get_modifier_mapping_reply(_wm.dpy, xcb_get_modifier_mapping(_wm.dpy), &err);
    if(err)
    {   free(reply);
        free(err);
        return;
    }

	xcb_keycode_t *codes = xcb_get_modifier_mapping_keycodes(reply);
	xcb_keycode_t target, *temp = NULL;
	unsigned int i, j;

    if(!(temp = xcb_key_symbols_get_keycode(_wm.syms, XK_Num_Lock)))
    {   free(reply);
        return;
    }

	target = *temp;
	free(temp);

	for(i = 0; i < 8; i++)
		for(j = 0; j < reply->keycodes_per_modifier; j++)
			if(codes[i * reply->keycodes_per_modifier + j] == target)
				_wm.numlockmask = (1 << i);
    free(reply);
}

void
updatesizehints(Client *c, XCBSizeHints *size)
{
    /* init values */
    c->basew = c->baseh = 0;
    c->incw = c->inch = 0;
    c->maxw = c->maxh = 0;
    c->minw = c->minh = 0;
    c->maxa = c->mina = 0.0;

    /* size is uninitialized, ensure that size.flags aren't used */
    size->flags += !size->flags * XCB_SIZE_HINT_P_SIZE;

    if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        c->minw = size->min_width;
        c->minh = size->min_height;
    }
    else if(size->flags & XCB_SIZE_HINT_P_BASE_SIZE)
    {
        c->minw = size->base_width;
        c->minh = size->base_height;
    }

    if(size->flags & XCB_SIZE_HINT_P_BASE_SIZE)
    {
        c->basew = size->base_width;
        c->baseh = size->base_height;
    }
    else if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        c->basew = c->minw;
        c->baseh = c->minh;
    }

    if(size->flags & XCB_SIZE_HINT_P_RESIZE_INC)
    {
        c->incw = size->width_inc;
        c->inch = size->height_inc;
    }
    if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        c->maxw = size->max_width;
        c->maxh = size->max_height;
    }
    if(size->flags & XCB_SIZE_HINT_P_ASPECT)
    {
        c->mina = (float)size->min_aspect_den / size->min_aspect_num;
        c->maxa = (float)size->max_aspect_num / size->max_aspect_den;
    }
}

void
updatetitle(Client *c)
{
}

void
updatewindowstate(Client *c, XCBAtom state, uint8_t add_remove_toggle)
{
    if(!c || !state)
    {   return;
    }
    const u8 toggle = add_remove_toggle == 2;
    /* This is similiar to those Windows 10 dialog boxes that play the err sound and cant click anything else */
    if (state == netatom[NetWMStateModal])
    {
        if(toggle)
        {   
            setmodal(c, !ISMODAL(c));
            setwtypedialog(c, !ISDIALOG(c));
        }
        else
        {
            setmodal(c, add_remove_toggle);
            setwtypedialog(c, add_remove_toggle);
        }
    }                                                           /* This is just syntax sugar, really its just a alias to NetWMStateAbove */
    else if (state == netatom[NetWMStateAbove] || state == netatom[NetWMStateAlwaysOnTop])
    {
        if(toggle)
        {
            setalwaysontop(c, !ISALWAYSONTOP(c));
        }
        else
        {
            setalwaysontop(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateDemandAttention])
    {
        if(toggle)
        {   
            seturgent(c, !ISURGENT(c));
        }
        else
        {
            seturgent(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateFullscreen])
    {
        if(toggle)
        {
            setfullscreen(c, !ISFULLSCREEN(c));
        }
        else
        {
            setfullscreen(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateMaximizedHorz])
    {
        /* toggle is already handled here */
        setmaximizedhorz(c, add_remove_toggle);
    }
    else if (state == netatom[NetWMStateMaximizedVert])
    {
        /* toggle is already handled here */
        setmaximizedhorz(c, add_remove_toggle);
    }
    else if (state == netatom[NetWMStateSticky])
    {
        if(toggle)
        {   
            setsticky(c, !ISSTICKY(c));
        }
        else
        {
            setsticky(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateBelow])
    {   
        /* this is a wierd state to even configure so idk */
        if(toggle)
        {   
            setalwaysonbottom(c, !ISALWAYSONBOTTOM(c));
        }
        else
        {
            /* attach last */
            setalwaysonbottom(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateSkipTaskbar])
    {   
        if(toggle)
        {
            setskiptaskbar(c, !SKIPTASKBAR(c));
        }
        else
        {
            setskiptaskbar(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateSkipPager])
    {
        if(toggle)
        {
            setskippager(c, !SKIPPAGER(c));
        }
        else
        {
            setskippager(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateHidden])
    {   
        if(toggle)
        {
            sethidden(c, !ISHIDDEN(c));
        }
        else
        {
            sethidden(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateFocused])
    {
        if(toggle)
        {
            SETFLAG(c->wstateflags, _STATE_FOCUSED, !ISFOCUSED(c));
        }
        else
        {
            SETFLAG(c->wstateflags, _STATE_FOCUSED, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateShaded])
    {
        if(toggle)
        {
            setshaded(c, !ISSHADED(c));
        }
        else
        {
            setshaded(c, add_remove_toggle);
        }
    }
    else
    {
        DEBUG0("Could not find state.");
    }
}

void
updatewindowstates(Client *c, XCBAtom states[], uint32_t atomslength)
{
    if(!states || !c)
    {   return;
    }

    /* bullshit client is trying to mess with us */
    u16 MAX_LIMIT = 1000;
    atomslength = MIN(atomslength, MAX_LIMIT);


    u32 i;
    for(i = 0; i < atomslength; ++i)
    {
        /* Even though the wm-spec says that we should remove things that arent in the list 
         * The client will ussually tell us in clientmessage if its important. 
         * It also says however that if its in the list assume its a prop so...
         */
        updatewindowstate(c, states[i], 1);
    }
}

void
updatewindowtype(Client *c, XCBAtom wtype, uint8_t add_remove_toggle)
{
    if(!c || !wtype)
    {   return;
    }

    const u8 toggle = add_remove_toggle == 2;

    if (wtype == netatom[NetWMWindowTypeDesktop])
    {
        if(toggle)
        {   
            setwtypedesktop(c, !ISDESKTOP(c));
        }
        else
        {
            setwtypedesktop(c, add_remove_toggle);
        }
        /* TODO */
    }
    else if (wtype == netatom[NetWMWindowTypeDock])
    {
        if(toggle)
        {
            setwtypedock(c, !ISDOCK(c));
        }
        else
        {
            setwtypedock(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeToolbar])
    {   
        if(toggle)
        {
            setwtypetoolbar(c, !ISTOOLBAR(c));
        }
        else
        {
            setwtypetoolbar(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeMenu])
    {
        if(toggle)
        {
            setwtypemenu(c, !ISMENU(c));
        }
        else
        {
            setwtypemenu(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeUtility])
    {
        if(toggle)
        {
            setwtypeutility(c, !ISUTILITY(c));
        }
        else
        {
            setwtypeutility(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeSplash])
    {
        if(toggle)
        {   
            setwtypesplash(c, !ISSPLASH(c));
        }
        else
        {
            setwtypesplash(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDialog])
    {   
        if(toggle)
        { 
            setwtypedialog(c, !ISDIALOG(c));
        }
        else
        {
            setwtypedialog(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDropdownMenu])
    {   
        if(toggle)
        { 
            setwtypedropdownmenu(c, !ISDROPDOWNMENU(c));
        }
        else
        {
            setwtypedropdownmenu(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypePopupMenu])
    {
        if(toggle)
        {
            setwtypepopupmenu(c, !ISPOPUPMENU(c));
        }
        else
        {
            setwtypepopupmenu(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeTooltip])
    {
        if(toggle)
        {
            setwtypetooltip(c, !ISTOOLTIP(c));
        }
        else
        {
            setwtypetooltip(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeNotification])
    { 
        if(toggle)
        {
            setwtypenotification(c, !ISNOTIFICATION(c));
        }
        else
        {
            setwtypenotification(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeCombo])
    {
        if(toggle)
        {
            setwtypecombo(c, !ISCOMBO(c));
        }
        else
        {
            setwtypecombo(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDnd])
    {
        if(toggle)
        {
            setwtypednd(c, !ISDND(c));
        }
        else
        {
            setwtypednd(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeNormal])
    {
        if(toggle)
        {
            setwtypenormal(c, !ISNORMAL(c));
        }
        else
        {
            setwtypenormal(c, add_remove_toggle);   
        }
    }
    else
    {
        DEBUG0("Could not find type.");
    }
}

void
updatewindowtypes(Client *c, XCBAtom wtypes[], uint32_t atomslength)
{
    if(!wtypes || !c)
    {   return;
    }
    /* bullshit client is trying to mess with us */
    u8 MAX_LIMIT = 255;
    atomslength = MIN(atomslength, MAX_LIMIT);

    i32 i;
    for(i = 0; i < atomslength; ++i)
    {   
        /* wm-spec says that we should assume anythings in the list are props so we just pass into "add" */
        updatewindowtype(c, wtypes[i], 1);
    }
}

void
updatewmhints(Client *c, XCBWMHints *wmh)
{
    if(wmh)
    {
        if(c == c->desktop->sel && wmh->flags & XCB_WM_HINT_URGENCY)
        {
            wmh->flags &= ~XCB_WM_HINT_URGENCY;
            XCBSetWMHintsCookie(_wm.dpy, c->win, wmh);
            /* dont put seturgent() here cause that would just undo what we did and be recursive */
        }
        else
        {   
            /* dont put seturgent() here cause that would just undo what we did and be recursive */
            SETFLAG(c->wstateflags, _STATE_DEMANDS_ATTENTION, !!(wmh->flags & XCB_WM_HINT_URGENCY));
        }
        if(wmh->flags & XCB_WM_HINT_INPUT)
        {   setneverfocus(c, !wmh->input);
        }
        else
        {   setneverfocus(c, 0);
        }
    }
}

void
wakeupconnection()
{
    if(!_wm.dpy)
    {   DEBUG0("No connection avaible");
        return;
    }
    XCBClientMessageEvent ev;
    memset(&ev, 0, sizeof(XCBClientMessageEvent));
    ev.type = wmatom[WMProtocols];
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = _wm.root;
    ev.format = 32;
    ev.data.data32[0] = wmatom[WMDeleteWindow];
    ev.data.data32[1] = XCB_CURRENT_TIME;
                                        /* XCB_EVENT_MASK_NO_EVENT legit does nothing lol */
    XCBSendEvent(_wm.dpy, _wm.root, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
    /* make sure display gets the event (duh) */
    XCBFlush(_wm.dpy);
}

void
winsetstate(XCBWindow win, i32 state)
{
    i32 data[] = { state, XCB_NONE };
    XCBChangeProperty(_wm.dpy, win, wmatom[WMState], wmatom[WMState], 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}

void *
wintobar(XCBWindow win, uint8_t getmon)
{
    Monitor *m = NULL;
    for(m = _wm.mons; m; m = nextmonitor(m))
    {
        if(m->bar && m->bar->win == win)
        {   
            if(getmon)
            {   return m;
            }
            else
            {   return m->bar;
            }
            break;
        }
    }
    return NULL;
}

Client *
wintoclient(XCBWindow win)
{
    Client *c = NULL;
    Desktop *desk = NULL;
    Monitor *m = NULL;

    for(m = _wm.mons; m; m = nextmonitor(m))
    {
        for(desk = m->desktops; desk; desk = nextdesktop(desk))
        {
            for(c = desk->clients; c; c = nextclient(c))
            {
                if(c->win == win)
                {   DEBUG("WINDOW: %d", win);
                    return c;
                }
            }
        }
    }
    return NULL;
}

Monitor *
wintomon(XCBWindow win)
{
    i16 x, y;
    Client *c;
    Monitor *m;
    if(win == _wm.root && getrootptr(&x, &y)) return recttomon(x, y, 1, 1);
    for (m = _wm.mons; m; m = m->next)
        if (m->bar && win == m->bar->win) return m;
    if ((c = wintoclient(win))) return c->desktop->mon;
    return _wm.selmon;
}

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(err)
    {   
        DEBUG("%s %s\n", XCBGetErrorMajorCodeText(err->major_code), XCBGetFullErrorText(err->error_code));
        DEBUG("error_code: [%d], major_code: [%d], minor_code: [%d]\n"
              "sequence: [%d], response_type: [%d], resource_id: [%d]\n"
              "full_sequence: [%d]\n"
              ,
           err->error_code, err->major_code, err->minor_code, 
           err->sequence, err->response_type, err->resource_id, 
           err->full_sequence);
    }
}

int
main(int argc, char *argv[])
{
    argcvhandler(argc, argv);
    startup();
    setup();
#ifdef __OpenBSD__
        if (pledge("stdio rpath proc exec", NULL) == -1)
            die("pledge");
#endif /* __OpenBSD__ */
    scan();
    run();
    cleanup();
    /* under special conditions do certain things */
    specialconds(argc, argv);
    return EXIT_SUCCESS;
}
