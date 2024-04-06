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

    if(width && !BETWEEN(*width, MIN_POSSIBLE_WINDOW_SIZE , ww))
    {
        if(*width < MIN_POSSIBLE_WINDOW_SIZE )
        {   *width = MIN_POSSIBLE_WINDOW_SIZE;
        }
        else
        {   *width = ww;
        }
    }

    if(height && !BETWEEN(*height, MIN_POSSIBLE_WINDOW_SIZE , wh))
    {
        if(*height < MIN_POSSIBLE_WINDOW_SIZE)
        {   *height = MIN_POSSIBLE_WINDOW_SIZE;
        }
        else
        {   *height = wh;
        }
    }

    if(x && !BETWEEN(*x, wx - ww, wx + ww))
    {
        if(*x < (wx - ww))
        {   *x = (wx - ww);
        }
        else
        {   *x = (wx + ww);
        }
        DEBUG0("Specified x is not in bounds, undesired behaviour may occur.");
    }

    if(y && !BETWEEN(*y, wy - wh, wy + wh))
    {
        if(*y < (wy - wh))
        {   *y = (wy - wh);
        }
        else
        {   *y = (wy + wh);
        }
        DEBUG0("Specified y is not in bounds, undesired behaviour may occur.");
    }
    const u8 NO_BORDER_WIDTH = 0;
    if(border_width && !BETWEEN(*border_width, NO_BORDER_WIDTH, ww))
    {
        if(*border_width < NO_BORDER_WIDTH)
        {   *border_width = NO_BORDER_WIDTH;
        }
        else
        {   *border_width = ww - MIN_POSSIBLE_WINDOW_SIZE;
        }
        DEBUG0("Border width seems to be too big.");
    }
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
    Monitor *m = c->mon;
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
    Desktop *workingdesk = NULL;

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
attachbar(Monitor *m, XCBWindow barwin)
{
    /* due to the infrequency of this we can afford multiple cycles wasted here */
    if(!barwin)
    {   
        DEBUG0("Cant Attach no bar to monitor, this shouldnt be possible");
        return;
    }
    if(!m)
    {   
        DEBUG0("Cant Attach barwin because the monitor is NULL, this shouldnt be possible");
        return;
    }
    if(m->bar && m->bar->win)
    {   
        if(m->bar && m->bar->win == barwin)
        {   
            DEBUG0("Cant Attach barwin to same barwin");
        }
        else
        {   
            DEBUG0("Cant Attach barwin because another barwin is already attached");
        }
    }
    else
    {
        DEBUG("Attached barwin: %d", barwin);
        Client *c = manage(barwin);
        if(!c)
        {   
            DEBUG0("Could not attach bar, for some reason...");
            return;
        }
        detachcompletely(c);
        c->flags = 0;
        m->bar = c;
    }
}

void
detachbar(Monitor *m)
{
    /* due to the infrequency of this we can afford multiple cycles wasted here */
    if(!m)
    {   DEBUG0("Cant Detach barwin as monitor is NULL");
        return;
    }
    cleanupclient(m->bar);
    m->bar = NULL;
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


void
cfgsethoverfocus(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _CFG_HOVERFOCUS, !!state);
}

u8
checknewbar(XCBWindow win)
{
    /* todo */
    return 0;
    u8 status = 0;

    XCBCookie clshintcookie = XCBGetPropertyCookie(_wm.dpy, win, wmatom[WMClass], 0, 64, 0, XCB_ATOM_ANY);

    return status;
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

void
cleanup(void)
{
    XCBCookie cookie = XCBDestroyWindow(_wm.dpy, _wm.wmcheckwin);
    XCBDiscardReply(_wm.dpy, cookie);
    if(_wm.syms)
    {   
        XCBKeySymbolsFree(_wm.syms);
        _wm.syms = NULL;
    }
    cleanupmons();
    XCBFlush(_wm.dpy);
    XCBCloseDisplay(_wm.dpy);
    _wm.dpy = NULL;
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
    XCBConfigureNotifyEvent ce;
    ce.response_type = XCB_CONFIGURE_NOTIFY;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above_sibling = XCB_NONE;
    ce.override_redirect = False;
    XCBSendEvent(_wm.dpy, c->win, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ce);
}

Client *
createclient(Monitor *m)
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
    c->flags = 0;
    c->bw = c->oldbw = 0;
    c->win = 0;
    c->mon = m;
    c->mina = c->maxa = 0;
    c->basew = c->baseh = 0;
    c->incw = c->inch = 0;
    c->maxw = c->maxh = 0;
    c->pid = 0;
    c->desktop = m->desksel;
    return c;
}

Desktop *
createdesktop(Monitor *m)
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
    attachdesktop(m, desk);
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
    m->bar = NULL;
    u16 i;
    /* TODO */
    for(i = 0; i < 10; ++i)
    {   createdesktop(m);
    }
    m->desksel = m->desktops;
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

uint8_t
docked(Client *c)
{
    uint8_t dockd = 0;
    if(c->mon)
    {   
        dockd = (c->mon->wx == c->x) & (c->mon->wy == c->y) & (WIDTH(c) == c->mon->ww) & (HEIGHT(c) == c->mon->wh);
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
        if(c->mon != _wm.selmon)
        {   _wm.selmon = c->mon;
        }

        if(ISURGENT(c))
        {   seturgent(c, 0);
        }
        /* make it first on the stack */
        detachstack(c);
        attachstack(c);
        grabbuttons(c->win, 1);
        
        if(c->desktop->lastfocused && c->desktop->lastfocused != c)
        {   /* set window border */
        }
        setfocus(c);
    }
    else
    {   
        XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
    }
    desk->sel = c;
}

i32
getstate(XCBWindow win, XCBGetWindowAttributes *state_att)
{
    i32 state = 0;
    const XCBCookie cookie = XCBGetWindowPropertyCookie(_wm.dpy, win, wmatom[WMState], 0L, 2L, False, wmatom[WMState]);
    XCBGetWindowAttributes *reply = XCBGetWindowAttributesReply(_wm.dpy, cookie);
    if(reply)
    {
        state = reply->map_state;
        free(reply);
    }
    return state;
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
    /* makesure to ungrab buttons first */
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
    Client *c;
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
    ch = desk->clients->mon->wh / (rows + !rows);
    cw = desk->clients->mon->ww / (cols + !cols);
    for(i = 0, c = nexttiled(desk->clients); c; c = nexttiled(c->next))
    {
        nx = desk->clients->mon->wx + (i / rows) * cw;
        ny = desk->clients->mon->wy + (i % rows) * ch;
        /* adjust height/width of last row/column's windows */
        ah = !!((i + 1) % rows) * (desk->clients->mon->wh - ch * rows);
        aw = !!(i >= rows * (cols - 1)) * (desk->clients->mon->ww - cw * cols);

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
        resize(c, nx, ny, nw, nh, 1);
        ++i;
    }
}

Client *
manage(XCBWindow win)
{
    Client *c, *t = NULL;
    XCBWindow trans = 0;
    u8 transstatus = 0;
    u32 inputmask = XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_FOCUS_CHANGE|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    XCBWindowGeometry *wg;

    /* checks */
    if(win == _wm.root)
    {   DEBUG("%s", "Cannot manage() root window.");
        return NULL;
    }
    else if(_wm.selmon->bar && _wm.selmon->bar->win == win)
    {
        DEBUG0("Cannot manage() bar window.");
        return NULL;
    }
    else if(checknewbar(win))
    {   
        managebar(_wm.selmon, win);
        return NULL;
    }


    /* get cookies first */
    XCBCookie wacookie = XCBGetWindowAttributesCookie(_wm.dpy, win);
    XCBCookie wgcookie    = XCBGetWindowGeometryCookie(_wm.dpy, win);
    XCBCookie transcookie = XCBGetTransientForHintCookie(_wm.dpy, win);                        
    XCBCookie wtypecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMWindowType], 0L, UINT32_MAX, False, XCB_ATOM_ATOM);
    XCBCookie statecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMState], 0L, UINT32_MAX, False, XCB_ATOM_ATOM);
    XCBCookie sizehcookie = XCBGetWMNormalHintsCookie(_wm.dpy, win);
    XCBCookie wmhcookie   = XCBGetWMHintsCookie(_wm.dpy, win);

    XCBGetWindowAttributes *waattributes;
    XCBWindowProperty *wtypeunused;
    XCBWindowProperty *stateunused;
    XCBSizeHints hints;
    u8 hintstatus = 0;
    XCBWMHints *wmh;

    c = createclient(_wm.selmon);
    c->win = win;

    /* wait for replies */
    waattributes = XCBGetWindowAttributesReply(_wm.dpy, wacookie);
    wg = XCBGetWindowGeometryReply(_wm.dpy, wgcookie);
    transstatus = XCBGetTransientForHintReply(_wm.dpy, transcookie, &trans);
    wtypeunused = XCBGetWindowPropertyReply(_wm.dpy, wtypecookie);
    stateunused = XCBGetWindowPropertyReply(_wm.dpy, statecookie);
    hintstatus = XCBGetWMNormalHintsReply(_wm.dpy, sizehcookie, &hints);
    wmh = XCBGetWMHintsReply(_wm.dpy, wmhcookie);

    /* On Failure clear flag and ignore hints */
    hints.flags *= !!hintstatus;    

    if(waattributes)
    {   
        if(waattributes->override_redirect)
        {   
            free(c);
            free(waattributes);
            free(wmh);
            free(stateunused);
            free(wtypeunused);
            free(wg);
            return NULL;
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
    }

    if(transstatus && trans && (t = wintoclient(trans)))
    {
        c->mon = t->mon;
        c->desktop = t->desktop;
    }
    else
    {
        c->mon = _wm.selmon;
        /* applyrules() */
    }

    /* constrain window to window area */
    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
    {   c->x = c->mon->wx + c->mon->ww - WIDTH(c);
    }
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
    {   c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
    }
    c->x = MAX(c->x, c->mon->wx);
    c->y = MAX(c->y, c->mon->wy);


    /* Custom stuff */
    setborderwidth(c, _cfg.bw);

    XCBSetWindowBorderWidth(_wm.dpy, win, c->bw);
    /*  XSetWindowBorder(dpy, w, scheme[SchemeBorder][ColBorder].pixel); */
    configure(c);   /* propagates border_width, if size doesn't change */
    updatetitle(c);
    updatesizehints(c, &hints);
    updatewmhints(c, wmh);
    XCBSelectInput(_wm.dpy, win, inputmask);
    grabbuttons(win, 0);

    setfloating(c, trans != XCB_NONE); /* this just covers a few other checks */
    setfloating(c, trans != XCB_NONE || ISALWAYSONTOP(c) || ISFLOATING(c));
    attach(c);
    attachstack(c);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&win, 1);
    setclientstate(c, XCB_WINDOW_NORMAL_STATE);
    /* map the window or we get errors */
    XCBMapWindow(_wm.dpy, win);

    if(c->mon == _wm.selmon)
    {   unfocus(_wm.selmon->desksel->sel, 0);
    }
    /* inherit previous client state */
    if(_wm.selmon->desksel->sel)
    {   setfullscreen(c, ISFULLSCREEN(_wm.selmon->desksel->sel) || ISFULLSCREEN(c));
    }
    _wm.selmon->desksel->sel = c;
    arrange(c->desktop);
    /* client could be floating so we pass NULL for focus */
    focus(NULL);
    /* reply cleanup */
    free(waattributes);
    free(wmh);
    free(stateunused);
    free(wtypeunused);
    free(wg);

    return c;
}

Client *
managebar(Monitor *m, XCBWindow win)
{
    if(!m->bar)
    {   m->bar = createclient(NULL);
    }

    if(!m->bar)
    {   return NULL;
    }
    memset(m->bar, 0, sizeof(Client));



    return m->bar;
}

void
monocle(Desktop *desk)
{
    if(!desk->clients)
    {   return;
    }
    Client *c;
    i32 nw, nh;
    i32 nx = desk->clients->mon->wx;
    i32 ny = desk->clients->mon->wy;

    for(c = nexttiled(desk->clients); c; c = nexttiled(c->next))
    {
        nw = desk->clients->mon->ww - (c->bw * 2);
        nh = desk->clients->mon->wh - (c->bw * 2);
        resize(c, nx, ny, nw, nh, 1); 
        if(docked(c))
        {   setfloating(c, 0);
        }
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
    if(desk->stack->mon->bar)
    {   wc.sibling = c->mon->bar->win;
    }
    else
    {   
        wc.sibling = c->win;
        c = nextstack(c);
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

    /* The XServer doesnt receive this request till the next event, so flush them to make sure it does */
    XCBFlush(_wm.dpy);
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

void
sendmon(Client *c, Monitor *m)
{
    if(c->mon == m)
    {   return;
    }
    unfocus(c, 1);
    detachcompletely(c);
    c->mon = m;
    c->desktop = m->desksel;
    attach(c);
    attachstack(c);
    focus(NULL);
    /* arrangeall() */
}

void
setalwaysontop(Client *c, u8 state)
{
    SETFLAG(c->flags, _ALWAYSONTOP, !!state);
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
    if(c->next || c->prev)
    {   detach(c);
    }
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
updatedesktop(void)
{
    i32 data[1] = { _wm.selmon->desksel->num };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetCurrentDesktop], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 1);
}

void
setdesktoplayout(Desktop *desk, uint8_t layout)
{
    desk->olayout = layout;
    desk->layout = layout;
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
setdialog(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _DIALOG, !!state);
}

void
setfixed(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _FIXED, !!state);
}

void
setfloating(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _WASFLOATING, !!ISFLOATING(c));
    SETFLAG(c->flags, _FLOATING, !!state);
}

void
setfullscreen(Client *c, u8 state)
{
    if(state && !ISFULLSCREEN(c))
    {
        XCBChangeProperty(_wm.dpy, c->win, netatom[NetWMState], XCB_ATOM_ATOM, 32,
        XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom[NetWMStateFullscreen], 1);
        setborderwidth(c, c->bw);
        setborderwidth(c, 0);
        resizeclient(c, c->mon->mx, c->mon->wy, c->mon->mw, c->mon->mh);
        XCBRaiseWindow(_wm.dpy, c->win);
    }
    else if(!state && ISFULLSCREEN(c))
    {
        XCBChangeProperty(_wm.dpy, c->win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
        XCB_PROP_MODE_REPLACE, (unsigned char *)0, 0);
        setborderwidth(c, c->oldbw);
        resizeclient(c, c->oldx, c->oldy, c->oldw, c->oldh);
    }
    SETFLAG(c->flags, _FULLSCREEN, !!state);
}

void
setfocus(Client *c)
{
    if(!NEVERFOCUS(c))
    {
        XCBSetInputFocus(_wm.dpy, c->win, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&(c->win), 1);
        XCBClientMessageEvent ev;
        ev.type = wmatom[WMProtocols];
        ev.response_type = XCB_CLIENT_MESSAGE;
        ev.window = c->win;
        ev.format = 32;
        ev.data.data32[0] = wmatom[WMTakeFocus];
        ev.data.data32[1] = XCB_CURRENT_TIME;
        XCBSendEvent(_wm.dpy, c->win, False, XCB_NONE, (const char *)&ev);
    }
}

void 
sethidden(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _HIDDEN, !!state);
}
void
setmodal(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _MODAL, !!state);
}

void
setneverfocus(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _NEVERFOCUS, !!state);
}

void 
setshowbar(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _SHOWBAR, !!state);
}

void
setsticky(Client *c, u8 state)
{
    const XCBWindow win = c->win;
    const XCBAtom replace = !!state * netatom[NetWMStateSticky];
    XCBChangeProperty(_wm.dpy, win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
            XCB_PROP_MODE_REPLACE, (unsigned char *)&replace, !!replace);

    SETFLAG(c->flags, _STICKY, !!state);
}

void 
settopbar(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _TOPBAR, !!state);
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

    if(!_wm.syms)
    {   DIECAT("%s", "Could not establish connection with keyboard (OutOfMemory)");
    }

    updatesettings();

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

    SETFLAG(c->flags, _URGENT, !!state);
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
updateviewport(void)
{
    i32 data[2] = { 0, 0 };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopViewport], 
            XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}


void
showhide(const Client *restrict c)
{
    if(ISVISIBLE(c))
    {   XCBMoveWindow(_wm.dpy, c->win, c->x, c->y);
    }
    else
    {   
        const i16 x = (c->mon->mx - (WIDTH(c) / 2));
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

    if(_wm.restart)
    {   execvp(argv[0], argv);
        /* UNREACHABLE */
        DEBUG("%s", "Failed to restart " NAME);
    }



    /* this is the end of the exithandler so we dont really care if we segfault here if at all.
     * But this covers some cases where system skips to here. (AKA manual interrupt)
     * Really this is mostly just to prevent XKeyboard saying we didnt free stuff. (its annoying)
     * Though it sometimes doesnt work, it works 90% of the time which is good enough.
     */
    if(_wm.dpy)
    {   cleanup();
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
    if(!_wm.dpy)
    {   DIECAT("%s", "FATAL: Cannot Connect to X Server.");
    }
    DEBUG("DISPLAY -> %s", display ? display : getenv("DISPLAY"));
    checkotherwm();
    XCBSetErrorHandler(xerror);
    /* This allows for execvp and exec to only spawn process on the specified display rather than the default varaibles */
    if(display)
    {   setenv("DISPLAY", display, 1);
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
    Client *c = NULL;
    Monitor *m = NULL;

    if(!desk->clients)
    {   return;
    }

    m = desk->clients->mon;
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
            nw = mw - (c->bw << 1);
            nh = h - (c->bw << 1);

            /* we divide nw also to get even gaps
             * if we didnt the center gap would be twices as big
             * Although this may be desired, one would simply remove the shift ">>" by 1 in nw 
             */
            nx += _cfg.bgw;
            ny += _cfg.bgw;
            nw -= _cfg.bgw << 1;
            nh -= _cfg.bgw << 1;
            resize(c, nx, ny, nw, nh, 1);
                                                                        /* spacing for windows below */
            if (my + HEIGHT(c) < (unsigned int)m->wh) my += HEIGHT(c) + _cfg.bgw;
        }
        else
        {
            h = (m->wh - ty) / (n - i);
            nx = m->wx + mw;
            ny = m->wy + ty;
            nw = m->ww - mw - (c->bw << 1);
            nh = h - (c->bw << 1);

            nx += _cfg.bgw >> 1;
            ny += _cfg.bgw;
            nw -= _cfg.bgw << 1;
            nh -= _cfg.bgw << 1;
            resize(c, nx, ny, nw, nh, 1);
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
    XCBSetWindowBorderWidth(_wm.dpy, c->win, 0);
    if(setfocus)
    {   
        XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
    }
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
				c->mon = _wm.mons;
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
updatebarpos(Monitor *m)
{
    m->wy = m->my;
    m->wh = m->mh;

    if(!m->bar)
    {   return;
    }
    if(SHOWBAR(m->bar))
    {
        m->wh -= m->bar->h;
        if(TOPBAR(m->bar))
        {
            m->bar->y = m->wy;
            m->wy += m->bar->h;
        }
        else
        {   
            m->bar->y = m->wy + m->wh;
        }
    }
    else
    {
        m->bar->y = -m->bar->h;
    }
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
updatesettings(void)
{
    _cfg.mfact = 0.55f;
    _cfg.nmaster = 1;
    _cfg.bw = 5;
    _cfg.bgw = 10;
    _cfg.snap = 10;
    _cfg.rfrate = 120;
    _cfg.bh = 10;
    _cfg.maxcc = 256;
    _cfg.wmname = "Gamer";
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

    if((c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh))
    {   setfixed(c, 1);
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
            setdialog(c, !ISDIALOG(c));
        }
        else
        {
            setmodal(c, add_remove_toggle);
            setdialog(c, add_remove_toggle);
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
    }
    else if (state == netatom[NetWMStateMaximizedVert])
    {
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
        }
        else
        {
            /* attach last */
            XCBLowerWindow(_wm.dpy, c->win);
        }
    }
    else if (state == netatom[NetWMStateSkipTaskbar])
    {   
    }
    else if (state == netatom[NetWMStateSkipPager])
    {
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
        if(c->desktop->sel != c)
        {   /* idk, we dont really care too much */
        }
    }
    else if (state == netatom[NetWMStateShaded])
    {
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
    Monitor *m = c->mon;
    if(!m)
    {   DEBUG0("This client doesnt have a monitor this shouldnt be possible");
        return;
    }

    const u8 toggle = add_remove_toggle == 2;

    if (wtype == netatom[NetWMWindowTypeDesktop])
    {
        if(toggle)   
        {
            setneverfocus(c, !NEVERFOCUS(c));
        }
        else
        {
            setneverfocus(c, add_remove_toggle);
        }
        /* TODO */
    }
    else if (wtype == netatom[NetWMWindowTypeDock])
    {
    }
    else if (wtype == netatom[NetWMWindowTypeToolbar])
    {   /* TODO */
    }
    else if (wtype == netatom[NetWMWindowTypeMenu])
    {   /* TODO */
    }
    else if (wtype == netatom[NetWMWindowTypeUtility])
    {   /* TODO */
    }
    else if (wtype == netatom[NetWMWindowTypeSplash])
    {   /* IGNORE */
    }
    else if (wtype == netatom[NetWMWindowTypeDialog])
    {   
        if(toggle)
        { 
            setdialog(c, !ISDIALOG(c));
        }
        else
        {
            setdialog(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDropdownMenu])
    {   
        if(toggle)
        { 
            setdialog(c, !ISDIALOG(c));
        }
        else
        {
            setdialog(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypePopupMenu])
    {   /* override-redirect IGNORE */
    }
    else if (wtype == netatom[NetWMWindowTypeTooltip])
    {   /* override-redirect IGNORE */
    }
    else if (wtype == netatom[NetWMWindowTypeNotification])
    {   /* override-redirect IGNORE */
    }
    else if (wtype == netatom[NetWMWindowTypeCombo])
    {   /* override-redirect IGNORE */
    }
    else if (wtype == netatom[NetWMWindowTypeDnd])
    {   /* override-redirect IGNORE */
    }
    else if (wtype == netatom[NetWMWindowTypeNormal])
    {   /* This hint indicates that this window has no special properties IGNORE */
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
            SETFLAG(c->flags, _URGENT, !!(wmh->flags & XCB_WM_HINT_URGENCY));
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
winsetstate(XCBWindow win, i32 state)
{
    i32 data[] = { state, XCB_NONE };
    XCBChangeProperty(_wm.dpy, win, wmatom[WMState], wmatom[WMState], 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
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
    if ((c = wintoclient(win))) return c->mon;
    return _wm.selmon;
}

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(err)
    {   
        DEBUG("%s %s\n", XCBErrorCodeText(err->error_code), XCBErrorMajorCodeText(err->major_code));
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
