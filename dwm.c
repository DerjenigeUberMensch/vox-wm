#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

#include <X11/cursorfont.h>     /* cursors */

/* keycodes */
#include <X11/keysym.h>

#include "util.h"
#include "dwm.h"
#include "parser.h"

#include "keybinds.h"
/* for HELP/DEBUGGING see under main() or the bottom */

extern void (*handler[XCBLASTEvent]) (XCBGenericEvent *);


WM _wm;
UserSettings _cfg;

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];
XCBAtom gtkatom[GTKLAST];
XCBAtom motifatom;
XCBCursor cursors[CurLast];



static uint32_t 
__intersect_area(
        /* rect 1 */
        int32_t x1, 
        int32_t y1, 
        int32_t x2, 
        int32_t y2,
        /* rect 2 */
        int32_t x3,
        int32_t y3,
        int32_t x4,
        int32_t y4
        )
{
    const int32_t xoverlap = MAX(0, MIN(x2, x4) - MAX(x1, x3));
    const int32_t yoverlap = MAX(0, MIN(y2, y4) - MAX(y1, y3));
    const uint32_t area = xoverlap * yoverlap;
    return area;
}

enum BarSides GETBARSIDE(Monitor *m, Client *bar, uint8_t get_prev)
                                { 
                                    const float LEEWAY = .15f;
                                    const float LEEWAY_SIDE = .35f;

                                    /* top parametors */
                                    const i32 TOP_MIN_X = m->mx;
                                    const i32 TOP_MIN_Y = m->my;

                                    const i32 TOP_MAX_X = m->mx + m->mw;
                                    const i32 TOP_MAX_Y = m->my + (m->mh * LEEWAY);

                                    /* bottom parametors */
                                    const i32 BOTTOM_MIN_X = m->mx;
                                    const i32 BOTTOM_MIN_Y = m->my + m->mh - (m->mh * LEEWAY);

                                    const i32 BOTTOM_MAX_X = m->mx + m->mw;
                                    const i32 BOTTOM_MAX_Y = m->my + m->mh;

                                    /* sidebar left parametors */
                                    const i32 LEFT_MIN_X = m->mx;
                                    const i32 LEFT_MIN_Y = TOP_MAX_Y;

                                    const i32 LEFT_MAX_X = m->mx + (m->mw * LEEWAY_SIDE);
                                    const i32 LEFT_MAX_Y = BOTTOM_MIN_Y;

                                    /* sidebar right parametors */
                                    const i32 RIGHT_MIN_X = m->mx + m->mw - (m->mw * LEEWAY_SIDE);
                                    const i32 RIGHT_MIN_Y = TOP_MAX_Y;

                                    const i32 RIGHT_MAX_X = m->mx + m->mw;
                                    const i32 RIGHT_MAX_Y = BOTTOM_MIN_Y;

                                    enum BarSides side;
                                    i32 bx1;
                                    i32 by1;
                                    i32 bx2;
                                    i32 by2;

                                    if(get_prev)
                                    {
                                        bx1 = bar->oldx + (bar->oldw / 2);
                                        by1 = bar->oldy + (bar->oldh / 2);
                                        bx2 = bx1 + bar->oldw;
                                        by2 = by1 + bar->oldh;
                                    }
                                    else
                                    {
                                        bx1 = bar->x + (bar->w / 2);
                                        by1 = bar->y + (bar->h / 2);
                                        bx2 = bx1 + bar->w;
                                        by2 = by1 + bar->h;
                                    }

                                    uint32_t toparea = __intersect_area(
                                            TOP_MIN_X,
                                            TOP_MIN_Y,
                                            TOP_MAX_X,
                                            TOP_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );
                                    uint32_t bottomarea = __intersect_area(
                                            BOTTOM_MIN_X,
                                            BOTTOM_MIN_Y,
                                            BOTTOM_MAX_X,
                                            BOTTOM_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );
                                    uint32_t leftarea = __intersect_area(
                                            LEFT_MIN_X,
                                            LEFT_MIN_Y,
                                            LEFT_MAX_X,
                                            LEFT_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );
                                    uint32_t rightarea = __intersect_area(
                                            RIGHT_MIN_X,
                                            RIGHT_MIN_Y,
                                            RIGHT_MAX_X,
                                            RIGHT_MAX_Y,
                                            bx1,
                                            by1,
                                            bx2,
                                            by2
                                            );

                                    uint32_t biggest = toparea;

                                    if(biggest < bottomarea)
                                    {   biggest = bottomarea;
                                    }

                                    if(biggest < leftarea)
                                    {   biggest = leftarea;
                                    }

                                    if(biggest < rightarea)
                                    {   biggest = rightarea;
                                    }

                                    /* prob should handle the rare change that the area would be the same as another,
                                     * But at that point we should just rework it to use buttonpress last pressed location.
                                     */
                                    if(biggest == toparea)
                                    {   side = BarSideTop;
                                    }
                                    else if(biggest == bottomarea)
                                    {   side = BarSideBottom;
                                    }
                                    else if(biggest == leftarea)
                                    {   side = BarSideLeft;
                                    }
                                    else if(biggest == rightarea)
                                    {   side = BarSideRight;
                                    }
                                    else    /* this is just for compiler ommiting warning */
                                    {   side = BarSideTop;
                                    }

                                    return side;
                                }

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


void 
arrangeq(Desktop *desk)
{
    arrangedesktop(desk);
}

void
arrange(Desktop *desk)
{
    /* bar stuff */
    updatebargeom(desk->mon);
    updatebarpos(desk->mon);

    reorder(desk);
    arrangeq(desk);
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
    if(layouts[desk->layout].arrange)
    {   layouts[desk->layout].arrange(desk);
    }
}

/* Macro helper */
#define __attach_helper(STRUCT, HEAD, NEXT, PREV, LAST)     do                                      \
                                                            {                                       \
                                                                STRUCT->NEXT = STRUCT->HEAD;        \
                                                                STRUCT->HEAD = STRUCT;              \
                                                                if(STRUCT->NEXT)                    \
                                                                {   STRUCT->NEXT->PREV = STRUCT;    \
                                                                }                                   \
                                                                else                                \
                                                                {   STRUCT->LAST = STRUCT;          \
                                                                }                                   \
                                                                /* prevent circular linked list */  \
                                                                STRUCT->PREV = NULL;                \
                                                            } while(0)
#define __attach_after(START, AFTER, NEXT, PREV, HEAD, LAST)        do                                  \
                                                                    {                                   \
                                                                        AFTER->NEXT = START->NEXT;      \
                                                                        AFTER->PREV = START;            \
                                                                        if(!START->NEXT)                \
                                                                        {   LAST = START;               \
                                                                        }                               \
                                                                        else                            \
                                                                        {   START->NEXT->PREV = AFTER;  \
                                                                        }                               \
                                                                        START->NEXT = AFTER;            \
                                                                    } while(0)
#define __attach_before(START, BEFORE, NEXT, PREV, HEAD, LAST, ATTACH)  do                                  \
                                                                        {                                   \
                                                                            if(!START->PREV)                \
                                                                            {   ATTACH(BEFORE);             \
                                                                            }                               \
                                                                            else                            \
                                                                            {                               \
                                                                                BEFORE->PREV = START->PREV; \
                                                                                BEFORE->NEXT = START;       \
                                                                                START->PREV->NEXT = BEFORE; \
                                                                                START->PREV = BEFORE;       \
                                                                            }                               \
                                                                        } while(0)

/* Too hard to implement */
/*
#define __detach_helper(TYPE, STRUCT, HEAD, NEXT, PREV, LAST)   do                                                              \
                                                                {                                                               \
                                                                    TYPE **tc;                                                  \
                                                                    for(tc = &STRUCT->HEAD; *tc && *tc != STRUCT; tc = &(*tc)->NEXT);   \
                                                                    *tc = STRUCT->NEXT;                                         \
                                                                    if(!(*tc))                                                  \
                                                                    {   STRUCT->LAST = STRUCT->PREV;                            \
                                                                    }                                                           \
                                                                    else if(STRUCT->NEXT)                                       \
                                                                    {   STRUCT->NEXT->PREV = STRUCT->PREV;                      \
                                                                    }                                                           \
                                                                    else if(STRUCT->PREV)                                       \
                                                                    {                                                           \
                                                                        STRUCT->LAST = STRUCT->PREV;                            \
                                                                        STRUCT->PREV->NEXT = NULL;                              \
                                                                    }                                                           \
                                                                    STRUCT->NEXT = NULL;                                        \
                                                                    STRUCT->PREV = NULL;                                        \
                                                                } while(0)
*/
void
attach(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->clients, next, prev, desktop->clast);
}

/* NOT RECOMMENDED AS REQUIRES MANUAL SETTING OF DESKTOP->NUM */
void
attachdesktop(Monitor *m, Desktop *desktop)
{
    desktop->mon = m;
    desktop->num = 0;
    __attach_helper(desktop, mon->desktops, next, prev, mon->desklast);
}

void
attachdesktoplast(Monitor *m, Desktop *desk)
{
    if(!m->desktops)
    {   
        attachdesktop(m, desk);
        return;
    }

    desk->mon = m;
    desk->next = NULL;
    desk->prev = m->desklast;
    m->desklast->next = desk;
    m->desklast = desk;
    desk->num = m->deskcount++;
}

void
attachstack(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->stack, snext, sprev, desktop->slast);
}

void
attachrestack(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->rstack, rnext, rprev, desktop->rlast);
}

void
attachfocus(Client *c)
{
    if(ISOVERRIDEREDIRECT(c))
    {   return;
    }
    __attach_helper(c, desktop->focus, fnext, fprev, desktop->flast);
}

void
attachfocusafter(Client *start, Client *after)
{
    if(ISOVERRIDEREDIRECT(after))
    {   return;
    }
    Desktop *desk = start->desktop;
    detachfocus(after);
    __attach_after(start, after, fnext, fprev, desk->focus, desk->slast);
}

void
attachfocusbefore(Client *start, Client *after)
{
    if(ISOVERRIDEREDIRECT(after))
    {   return;
    }
    Desktop *desk = start->desktop;
    detachfocus(after);
    __attach_before(start, after, fnext, fprev, desk->focus, desk->slast, attachfocus);
}

void
detach(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->clients)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
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
    detachfocus(c);
    detachrestack(c);
}

void
detachdesktop(Monitor *m, Desktop *desktop)
{
    Desktop **td;
    for(td = &m->desktops; *td && *td != desktop; td = &(*td)->next);
    *td = desktop->next;
    if(!(*td))
    {
        m->desklast = desktop->prev;
    }
    else if(desktop->next)
    {
        desktop->next->prev = desktop->prev;
    }
    else if(desktop->prev)
    {
        m->desklast = desktop->prev;
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
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->stack)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Desktop *desk = c->desktop;
    Client **tc;

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
    c->sprev = NULL;
    c->snext = NULL;
}

void
detachrestack(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->rstack)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Desktop *desk = c->desktop;
    Client **tc;

    for(tc = &desk->rstack; *tc && *tc != c; tc = &(*tc)->rnext);
    *tc = c->rnext;
    if(!(*tc))
    {
        desk->rlast = c->rprev;
    }
    else if(c->rnext)
    {   
        c->rnext->rprev = c->rprev;
    }
    else if(c->rprev)
    {   /* this should be UNREACHABLE but if it does this should suffice */
        desk->rlast = c->rprev;
        c->rprev->rnext = NULL;
    }

    c->rprev = NULL;
    c->rnext = NULL;
}

void
detachfocus(Client *c)
{
    if(!c)
    {   DEBUG0("No client to detach FIXME");
        return;
    }
    if(!c->desktop)
    {   DEBUG0("No desktop in client FIXME");
        return;
    }
    if(!c->desktop->focus)
    {   DEBUG0("No clients in desktop FIXME");
        return;
    }
    Desktop *desk = c->desktop;
    Client **tc, *t;

    for(tc = &desk->focus; *tc && *tc != c; tc = &(*tc)->fnext);
    *tc = c->fnext;
    if(!(*tc))
    {
        desk->flast = c->fprev;
    }
    else if(c->fnext)
    {   
        c->fnext->fprev = c->fprev;
    }
    else if(c->fprev)
    {   /* this should be UNREACHABLE but if it does this should suffice */
        desk->flast = c->fprev;
        c->fprev->fnext = NULL;
    }

    /* this just updates desktop->sel */
    if (c == c->desktop->sel)
    {
        for (t = c->desktop->focus; t && !ISVISIBLE(t); t = t->fnext);
        c->desktop->sel = t;
    }

    c->fprev = NULL;
    c->fnext = NULL;
}

uint8_t
checknewbar(Monitor *m, Client *c, uint8_t has_strut_or_strutp)
{
    /* barwin checks */
    u8 checkbar = !m->bar;
    if(checkbar && COULDBEBAR(c, has_strut_or_strutp))
    {   
        setupbar(m, c);
        return 0;
    }
    return 1;
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
    return (x & 0xFFFFFFFF) | ((uint32_t)x == UINT32_MAX) | ((unsigned int)x == ~0)
        /* probably not but failsafe's */
    | ((uint32_t)x == (uint32_t)~0) | ((int32_t)x == -1) | ((uint32_t)x == (uint32_t) -1);
}

void
cleanup(void)
{
    savesession();
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
    cleanupcursors();
    XCBDiscardReply(_wm.dpy, cookie);
    XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
    XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
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
    free(desk->settings);
    free(desk);
    desk = NULL;
}

void
cleanupcursors(void)
{
    int i;
    for(i = 0; i < CurLast; ++i) 
    {   XCBFreeCursor(_wm.dpy, cursors[i]); 
    }
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
    HASH_CLEAR(hh, m->__hash);
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

Desktop *
createdesktop(void)
{
    Desktop *desk = calloc(1, sizeof(Desktop));
    if(!desk)
    {
        DEBUG("%s", "WARN: FAILED TO CREATE DESKTOP");
        return NULL;
    }
    desk->layout = 0;
    desk->olayout= 0;
    desk->clients= NULL;
    desk->clast = NULL;
    desk->stack = NULL;
    desk->slast = NULL;
    desk->mon = NULL;
    desk->settings = calloc(1, sizeof(UserSettings));
    if(!desk->settings)
    {   
        DEBUG("%s", "WARN: FAILED TO CREATE SETTINGS.");
        free(desk);
        return NULL;
    }
    return desk;
}

Monitor *
createmon(void)
{
    Monitor *m = calloc(1, sizeof(Monitor ));
    if(!m)
    {   /* while calling "DIE" may seem extreme frankly we cannot afford a monitor to fail alloc. */
        DIE("%s", "(OutOfMemory) Could not alloc enough memory for a Monitor");
        return NULL;
    }
    m->mx = m->my = 0;
    m->mw = m->mh = 0;
    m->wx = m->wy = 0;
    m->ww = m->wh = 0;
    m->next = NULL;
    /* FIXME: for some reason this cant be 0 because then we would get 2 "0" desktop nums */
    m->deskcount = 1;
    setdesktopcount(m, 10);
    m->desksel = m->desktops;
    m->bar = NULL;
    m->__hash = NULL;
    return m;
}

Decoration *
createdecoration(void)
{                       /* replace with malloc later... */
    Decoration *decor = calloc(1, sizeof(Decoration));
    /* TODO */
    if(decor)
    {
        decor->w = 0;
        decor->h = 0;
        decor->win = 0;
    }
    return decor;
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

void
eventhandler(XCBGenericEvent *ev)
{
    /* int for speed */
    const int cleanev = XCB_EVENT_RESPONSE_TYPE(ev);
    /* DEBUG("%s", XCBGetEventName(cleanev)); */
    if(LENGTH(handler) > cleanev)
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
getnamefromreply(XCBWindowProperty *namerep, char **str_return)
{
    if(namerep)
    {
        if(namerep->type && namerep->length > 0)
        {
            const size_t offset = XCBGetPropertyValueSize(namerep);
            char *str = XCBGetPropertyValue(namerep);
            char *nstr = malloc(sizeof(char) * offset + sizeof(char));
            if(nstr)
            {   
                memcpy(nstr, str, offset);
                memcpy(nstr + offset, "\0", sizeof(char));
            }
            *str_return = nstr;
            DEBUG("%s", *str_return);
        }
    }
}

u32 *
geticonprop(XCBWindowProperty *iconreply)
{
    const u8 WIDTH_INDEX = 0;
    const u8 HEIGHT_INDEX = 1;
    const u8 MIN_WIDTH = 1;
    const u8 MIN_HEIGHT = 1;
    const u8 MAX_WIDTH = 255;
    const u8 MAX_HEIGHT = 255;
    const u8 MIN_ICON_DATA_SIZE = (MIN_WIDTH + MIN_HEIGHT) * 2;     /* times 2 cause the first and second index are the size */

    u32 *ret = NULL;
    if(iconreply)
    {
        if(iconreply->format == 0)
        {   
            if(XCBGetPropertyValueSize(iconreply))
            {   DEBUG0("Icon has no format, icon may be corrupt.");
            }
            else
            {   DEBUG0("No icon.");
            }
            return ret;
        }
        if(iconreply->format != 32)
        {   DEBUG("Icon format is not standard, icon may be corrupt. %d", iconreply->format);
        }
        u32 *icon = XCBGetPropertyValue(iconreply);
        size_t size = XCBGetPropertyValueSize(iconreply);
        u32 length = XCBGetPropertyValueLength(iconreply, sizeof(u32));
        if(length >= MIN_ICON_DATA_SIZE)
        {   
            u64 i = 0;
            u64 wi = WIDTH_INDEX;
            u32 hi = HEIGHT_INDEX;
            /* get the biggest size */
            while(i + 2 < length)
            {                                               /* bounds check */
                if(icon[i + WIDTH_INDEX] > icon[wi] && icon[i + WIDTH_INDEX] <= MAX_WIDTH && length - i >= icon[i])
                {   wi = i;
                }
                                                                            /* bounds check */
                if(icon[i + HEIGHT_INDEX] > icon[hi] && icon[i + HEIGHT_INDEX] <= MAX_HEIGHT && length - i >= icon[i + 1])
                {   hi = i + HEIGHT_INDEX;
                }
                i += icon[i] + icon[i + 1];
                /* this covers use fucking up and the +plus 2 for width and height offset */
                i += 2;
            }
            size_t sz = sizeof(u32) * icon[wi] * icon[hi] + sizeof(u32) * 2;
            ret = malloc(MIN(sz, size));
            if(ret)
            {   memcpy(ret, &icon[wi], MIN(sz, size));
            }
        }
    }
    return ret;
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
grid(Desktop *desk)
{
    const float bgwr = USGetGapRatio(&_cfg);

    i32 n, cols, rows, cn, rn, i, cx, cy, cw, ch;
    i32 nx, ny;
    i32 nw, nh;
	Client *c;
    Monitor *m = desk->mon;

	for(n = 0, c = nexttiled(desk->stack); c; c = nexttiled(c->snext), n++);
	if(n == 0)
    {   return;
    }

	/* grid dimensions */
	for(cols = 0; cols <= n/2; cols++)
    {   
        if(cols*cols >= n)
        {   break;
        }
    }
    /* set layout against the general calculation: not 1:2:2, but 2:3 */
	if(n == 5) 
    {   cols = 2;
    }
	rows = n / cols;

	/* window geometries */
    cw = m->ww / (cols + !cols);
    /* current column number */
	cn = 0;
    /* current row number */
	rn = 0; 
	for(i = 0, c = nexttiled(desk->stack); c; i++, c = nexttiled(c->snext)) 
    {
		if((i / rows) + 1 > cols - (n % cols))
        {   rows = n/cols + 1;
        }
        ch = m->wh / (rows + !rows);
		cx = m->wx + cn * cw;
		cy = m->wy + rn * ch;

        nx = cx;
        ny = cy;
        nw = cw - (WIDTH(c) - c->w);
        nh = ch - (HEIGHT(c) - c->h);

        nx += (nw - (nw * bgwr)) / 2;
        ny += (nh - (nh * bgwr)) / 2;
        nw *= bgwr;
        nh *= bgwr;

		resize(c, nx, ny, nw, nh, 0);
        ++rn;
		if(rn >= rows) 
        {
			rn = 0;
            ++cn;
		}
	}
}

void
monocle(Desktop *desk)
{
    Client *c;
    Monitor *m = desk->mon;
    i32 nw, nh;
    const i32 nx = m->wx;
    const i32 ny = m->wy;

    for(c = nexttiled(desk->stack); c; c = nexttiled(c->snext))
    {
        nw = m->ww - (c->bw * 2);
        nh = m->wh - (c->bw * 2);
        resize(c, nx, ny, nw, nh, 0);
    }
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

Desktop *
prevdesktop(Desktop *desk)
{
    return desk ? desk->prev : desk;
}

void
quit(void)
{
    _wm.running = 0;
    wakeupconnection(_wm.dpy, _wm.screen);
}

static u8
_restore_parser(FILE *file, char *buffer, u16 bufflen)
{
    char *newline = NULL;
    const u8 success = 0;
    const u8 bufftoosmall = 1;
    const u8 error = 2;
    const u8 eof = 3;
    if(fgets(buffer, bufflen, file))
    {
        newline = strchr(buffer, '\n');
        if(!newline)
        {
            if(!feof(file))
            {   
                DEBUG0("Buffer too small for current line.");
                /* BUFF TOO SMALL */
                return bufftoosmall;
            }
        } /* remove new line char */
        else
        {   *newline = '\0';
        }
        /* Success. */
        return success;
    }
    if(ferror(file))
    {   /* ERROR */
        return error;
    }
    /* EOF */
    return eof;
}

void
restoresession(void)
{
    Monitor *m = NULL;
    Desktop *desk = NULL;

    FILE *fr = fopen(SESSION_FILE, "r");
    const int MAX_LENGTH = 1024;
    char str[MAX_LENGTH];
    int output = 0;
    u8 ismon = 0;
    u8 isdesk = 0;
    u8 isclient = 0;
    u8 isclients = 0;
    u8 isclientsend = 0;

    if(!fr)
    {   return;
    }

    enum Causes
    {
        NoError = 0,
        BuffTooSmall = 1,
        err = 2,
        EndOfFile = 3
    };
    while(output != EndOfFile)
    {
        memset(str, 0, MAX_LENGTH);
        output = _restore_parser(fr, str, MAX_LENGTH);
        switch(output)
        {
            case NoError: break;
            case BuffTooSmall:
            case err:
            case EndOfFile: 
            default: continue;
        }

        if(isclient)
        {   
            if(!desk)
            {   continue;
            }
            restoreclientsession(desk, str, MAX_LENGTH);
            isclient = 0;
            isclients = 0;
        }
        if(isclients)
        {
            if(!desk)
            {   continue;
            }
            if(!isclientsend)
            {
                const u8 SSCANF_CHECK_SUM = 1;
                XCBWindow win = 0;
                u8 status = sscanf(str, "Client: %u", &win);
                if(status == SSCANF_CHECK_SUM)
                {   
                    Client *c;
                    if((c = wintoclient(win)))
                    {   
                        setclientdesktop(c, desk);
                        DEBUG("Moving [%u] (Client) to right desktop...", c->win);
                    }
                    else
                    {   DEBUG0("Could not find client in stack...");
                    }
                }
                else
                {   /* TODO: Technically we dont need isclientsend, but having that prevents a "fail" due to strcmp("Client.", buff); happening after/ */
                    DEBUG0("Failed to pass move checksum for client.");
                }
            }
        }
        if(isdesk)
        {   
            if(!m)
            {   continue;
            }
            desk = restoredesktopsession(m, str, MAX_LENGTH);
            isdesk = 0;
        }

        if(ismon)
        {   
            m = restoremonsession(str, MAX_LENGTH);
            ismon = 0;
        }

        ismon += !strcmp(str, "Monitor.");
        isdesk += !strcmp(str, "Desktop.");
        isclient += !strcmp(str, "Client.");
        isclients += !strcmp(str, "Clients.");
        isclientsend += !strcmp(str, "ClientsEnd.");
    }
    fclose(fr);
    /* map all the windows again */
    Client *c;
    for(m = _wm.mons; m; m = nextmonitor(m))
    {
        for(desk = m->desktops; desk; desk = nextdesktop(desk))
        {
            for(c = desk->clients; c; c = nextclient(c))
            {   XCBMapWindow(_wm.dpy, c->win);
            }
        }
    }
    focus(NULL);
    arrangemons();
    /* No need to flush run() syncs for us */
    /* XCBFlush(_wm.dpy) */
}

Client *
restoreclientsession(Desktop *desk, char *buff, u16 len)
{
    const u8 SCANF_CHECK_SUM = 16;
    u8 check = 0;

    int x, y;
    int ox, oy;
    unsigned int w, h;
    unsigned int ow, oh;
    XCBWindow WindowId;
    XCBWindow WindowIdFocus;
    XCBWindow WindowIdStack;
    u32 Flags;
    u32 WTFlags;
    u32 WSFlags;
    u32 BorderWidth;
    u32 BorderColor;

    x = y = ox = oy = w = h = ow = oh = WindowId = WindowIdFocus = WindowIdStack = BorderWidth = BorderColor = 0;

    check = sscanf(buff, 
                    "(x: %d, y: %d) (w: %u h: %u)" " "
                    "(ox: %d, oy: %d) (ow: %u oh: %u)" " "
                    "WindowId: %u" " "
                    "WindowIdFocus: %u" " "
                    "WindowIdStack: %u" " "
                    "BorderWidth: %u" " "
                    "BorderColor: %u" " "
                    "Flags: %u" " "
                    "WTFlags: %u" " "
                    "WSFlags: %u" " "
                    ,
                    &x, &y, &w, &h,
                    &ox, &oy, &ow, &oh,
                    &WindowId,
                    &WindowIdFocus,
                    &WindowIdStack,
                    &BorderWidth,
                    &BorderColor,
                    &Flags,
                    &WTFlags,
                    &WSFlags
                    );

    Client *cclient = NULL;
    if(check == SCANF_CHECK_SUM)
    {
        cclient = wintoclient(WindowId);
        Client *fclient = wintoclient(WindowIdFocus);
        Client *sclient = wintoclient(WindowIdStack);
        if(cclient)
        {
            setborderwidth(cclient, BorderWidth);
            setbordercolor32(cclient, BorderColor);
            resize(cclient, ox, oy, ow, oh, 0);
            resize(cclient, x, y, w, h, 0);
            cclient->flags = Flags;
            cclient->wtypeflags = WTFlags;
            cclient->wstateflags = WSFlags;
        }
        if(fclient)
        {
            detachfocus(fclient);
            attachfocus(fclient);
        }
        if(sclient)
        {   
            detachstack(sclient);
            attachstack(sclient);
        }
    }
    else
    {   DEBUG0("FAILED CHECKSUM");
    }

    if(cclient)
    {   DEBUG("Restored Client: [%u]", cclient->win);
    }
    else if(check != SCANF_CHECK_SUM)
    {   DEBUG("Failed to parse Client str: \"%s\"", buff);
    }
    else
    {   DEBUG("Client Not Found: [%u]", WindowId);
    }
    return cclient;
}

Desktop *
restoredesktopsession(Monitor *m, char *buff, u16 len)
{
    const u8 SCANF_CHECK_SUM = 4;
    u8 check = 0;

    unsigned int DesktopLayout;
    unsigned int DesktopOLayout;
    unsigned int DesktopNum;
    XCBWindow DesktopSel;
    DesktopLayout = DesktopOLayout = DesktopNum = DesktopSel = 0;

    check = sscanf(buff, 
                    "DesktopLayout: %u" " "
                    "DesktopOLayout: %u" " "
                    "DesktopNum: %u" " "
                    "DesktopSel: %u" " "
                    ,
                    &DesktopLayout,
                    &DesktopOLayout,
                    &DesktopNum,
                    &DesktopSel
                    );

    if(check == SCANF_CHECK_SUM)
    {
        if(DesktopNum > m->deskcount)
        {   setdesktopcount(m, DesktopNum + 1);
        }
        u16 i;
        Desktop *desk = m->desktops;
        for(i = 0; i < DesktopNum; ++i)
        {   desk = nextdesktop(desk);
        }
        if(desk)
        {
            Client *sel = wintoclient(DesktopSel);

            if(sel && sel->desktop != desk)
            {   setclientdesktop(sel, desk);
            }
            focus(sel);
            setdesktoplayout(desk, DesktopOLayout);
            setdesktoplayout(desk, DesktopLayout);
        }
        DEBUG("Restored desktop: [%d]", desk ? desk->num : -1);
        return desk;
    }
    else
    {   DEBUG("Failed to parse Desktop str: \"%s\"", buff);
    }
    return NULL;
}

Monitor *
restoremonsession(char *buff, u16 len)
{
    const u8 SCANF_CHECK_SUM = 7;
    u8 check = 0;

    int x;
    int y;
    unsigned int h;
    unsigned int w;
    XCBWindow BarId;
    unsigned int DeskCount;
    unsigned int DeskSelNum;

    x = y = h = w = BarId = DeskCount = DeskSelNum = 0;

    check = sscanf(buff,
                    "(x: %d, y: %d) (w: %u h: %u)" " "
                    "BarId: %u" " "
                    "DeskCount: %u" " "
                    "DeskSelNum: %u" " "
                    ,
                    &x, &y, &w, &h,
                    &BarId,
                    &DeskCount,
                    &DeskSelNum
                    );
    if(check == SCANF_CHECK_SUM)
    {
        Monitor *pullm = NULL;
        Monitor *target = NULL;
        const u8 errorleeway = 5;
        Monitor *possible[errorleeway];
        int i;
        for(i = 0; i < errorleeway; ++i) { possible[i] = NULL; }
        u8 possibleInterator = 0;

        for(pullm = _wm.mons; pullm; pullm = nextmonitor(pullm))
        {
            if(pullm->mw == w && pullm->mh == h)
            {
                if(pullm->mx == x && pullm->my == y)
                {   target = pullm;
                    break;
                }
                else if(possibleInterator < errorleeway)
                {   possible[possibleInterator++] = pullm;
                }
            }
        }
        for(pullm = _wm.mons; pullm; pullm = nextmonitor(pullm))
        {
            if(BETWEEN(w, pullm->mw + errorleeway, pullm->mh - errorleeway) && BETWEEN(h, pullm->mh + errorleeway, pullm->mh - errorleeway))
            {
                if(possibleInterator < errorleeway)
                {   possible[possibleInterator++] = pullm;
                }
            }
        }
        pullm = target;
        if(!pullm) 
        {
            for(i = 0; i < errorleeway; ++i)
            {
                if(possible[i])
                {
                    if((pullm = recttomon(possible[i]->mx, possible[i]->my, possible[i]->mw, possible[i]->mh)))
                    {   break;
                    }
                }
            }
            if(!pullm)
            {   pullm = possible[0];
            }
        }
        if(pullm)
        {
            Client *b = wintoclient(BarId);
            if(b)
            {   
                if(pullm->bar && pullm->bar != b)
                {   
                    XCBWindow win = pullm->bar->win;
                    unmanage(pullm->bar, 0);
                    XCBCookie cookies[ManageCookieLAST];
                    managerequest(win, cookies);
                    managereply(win, cookies);
                }
                setupbar(pullm, b);
            }
            /* TODO */
            setdesktopcount(pullm, DeskCount);
            if(DeskSelNum != pullm->desksel->num)
            {
                Desktop *desk;
                for(desk = pullm->desktops; desk && desk->num != DeskSelNum; desk = nextdesktop(desk));
                if(desk)
                {   setdesktopsel(pullm, desk);
                }
            }
            DEBUG("Restored Monitor: [%p]", (void *)pullm);
        }
        return pullm;
    }
    else
    {   DEBUG("Failed to parse Monitor str: \"%s\"", buff);
    }
    return NULL;
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

void
restack(Desktop *desk)
{
    XCBWindowChanges wc;

    wc.stack_mode = XCB_STACK_MODE_BELOW;

    if(desk->mon->bar && !ISHIDDEN(desk->mon->bar))
    {   wc.sibling = desk->mon->bar->win;
    }
    else
    {   /* TODO: Maybe use wc.sibling = _wm.root? That causes error to be generated though. */
        wc.sibling = _wm.wmcheckwin;
    }

    Client *c;
    Client *cprev;
    u8 config = 0;
    u8 instack = 0;

    c = desk->stack;
    cprev = NULL;
    while(c)
    {
        instack = c->rprev || c->rnext;
        /* Client holds both lists so we just check if the next's are the same if not configure it */
        config = c->rnext != c->snext || !instack;
        if(config)
        {   
            XCBConfigureWindow(_wm.dpy, c->win, XCB_CONFIG_WINDOW_SIBLING|XCB_CONFIG_WINDOW_STACK_MODE, &wc);
            DEBUG("Configured window: %s", c->netwmname);
        }
        wc.sibling = c->win;
        /* apply reorder without detaching/attaching */
        cprev = c;
        c = nextstack(c);
        cprev->rprev = cprev->sprev;
        cprev->rnext = cprev->snext;
    }
    desk->rstack = desk->stack;
    desk->rlast = cprev;
}

void
reorder(Desktop *desk)
{
    updatestackpriorityfocus(desk);
    MERGE_SORT_LINKED_LIST(Client, stackpriority, desk->stack, desk->slast, snext, sprev, 1, 0);
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
    while(_wm.running && !XCBNextEvent(_wm.dpy, &ev))
    {
        eventhandler(ev);
        free(ev);
        ev = NULL;
    }
    _wm.has_error = XCBCheckDisplayError(_wm.dpy);
}

void
savesession(void)
{
    /* user settings */
    const char *filename = SESSION_FILE;
    Monitor *m;
    FILE *fw = fopen(filename, "w");
    if(!fw)
    {   DEBUG0("Failed to alloc FILE(OutOfMemory)");
        return;
    }

    for(m = _wm.mons; m; m = nextmonitor(m))
    {   savemonsession(fw, m);
    }
    fclose(fw);
}

void
saveclientsession(FILE *fw, Client *c)
{
    const char *IDENTIFIER = "Client.";
    static Client *c1 = NULL;
    static Client *c2 = NULL;
    XCBWindow focus;
    XCBWindow stack;

    if(!c1)
    {   c1 = c->desktop->flast;
    }
    if(!c2)
    {   c2 = c->desktop->slast;
    }

    focus = c1 ? c1->win : 0;
    stack = c2 ? c2->win : 0;

    fprintf(fw,
            "%s" 
            "\n"
            "(x: %d, y: %d) (w: %u h: %u)" " "
            "(ox: %d, oy: %d) (ow: %u oh: %u)" " "
            "WindowId: %u" " "
            "WindowIdFocus: %u" " "
            "WindowIdStack: %u" " "
            "BorderWidth: %u" " "
            "BorderColor: %u" " "
            "Flags: %u" " "
            "WTFlags: %u" " "
            "WSFlags: %u" " "
            "\n"
            ,
            IDENTIFIER,
            c->x, c->y, c->w, c->h,
            c->oldx, c->oldy, c->oldw, c->oldh,
            c->win,
            focus,
            stack,
            c->bw,
            c->bcol,
            c->flags,
            c->wtypeflags,
            c->wstateflags
            );

    if(c1 && prevfocus(c1))
    {   c1 = prevfocus(c1);
    }
    if(c2 && prevstack(c2))
    {   c2 = prevstack(c2);
    }
}

void
savedesktopsession(FILE *fw, Desktop *desk)
{
    const char *IDENTIFIER = "Desktop.";
    const char *IDENTIFIERCLIENTS = "Clients.";
    const char *IDENTIFIERCLIENTSEND = "ClientsEnd.";
    Client *c;

    fprintf(fw,
            "%s"
            "\n"
            "DesktopLayout: %u" " "
            "DesktopOLayout: %u" " "
            "DesktopNum: %u" " "
            "DesktopSel: %u" " "
            "\n"
            ,
            IDENTIFIER,
            desk->layout,
            desk->olayout,
            desk->num,
            desk->sel ? desk->sel->win : 0
            );
    if(desk->clients)
    {
        fprintf(fw, "%s\n", IDENTIFIERCLIENTS);
        for(c = desk->clients; c; c = nextclient(c))
        {   fprintf(fw, "Client: %u\n", c->win);
        }
    }
    fprintf(fw, "%s\n", IDENTIFIERCLIENTSEND);
    for(c = desk->clast; c; c = prevclient(c))
    {   saveclientsession(fw, c); 
    }
}

void
savemonsession(FILE *fw, Monitor *m) 
{
    const char *IDENTIFIER = "Monitor.";
    Desktop *desk;
    fprintf(fw,
            "%s"
            "\n"
            "(x: %d, y: %d) (w: %u h: %u)" " "
            "BarId: %u" " "
            "DeskCount: %u" " "
            "DeskSelNum: %u" " "
            "\n"
            ,
            IDENTIFIER,
            m->mx, m->my, m->mw, m->mh,
            m->bar ? m->bar->win : 0,
            m->deskcount,
            m->desksel->num
            );
    for(desk = m->desktops; desk; desk = nextdesktop(desk))
    {   savedesktopsession(fw, desk);
    }
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
            const size_t countsz = sizeof(XCBCookie ) * num;
            const size_t managecookiesz = sizeof(XCBCookie) * ManageCookieLAST;
            XCBCookie *wa = malloc(countsz);
            XCBCookie *wastates = malloc(countsz);
            XCBCookie *tfh = malloc(countsz);
            XCBCookie *managecookies = malloc(managecookiesz * num);
            XCBGetWindowAttributes **replies = malloc(sizeof(XCBGetWindowAttributes *) * num);
            XCBGetWindowAttributes **replystates = malloc(sizeof(XCBGetWindowAttributes *) * num);
            XCBWindow *trans = malloc(sizeof(XCBWindow) * num);

            if(!wa || !wastates || !tfh || !managecookies || !replies || !replystates || !trans)
            {   
                free(wa);
                free(wastates);
                free(tfh);
                free(managecookies);
                free(replies);
                free(replystates);
                free(trans);
                free(tree);
                return;
            }
            for(i = 0; i < num; ++i)
            {   
                wa[i] = XCBGetWindowAttributesCookie(_wm.dpy, wins[i]);
                /* this specifically queries for the state which wa[i] might fail to provide */
                wastates[i] = XCBGetWindowPropertyCookie(_wm.dpy, wins[i], wmatom[WMState], 0L, 2L, False, wmatom[WMState]);
                tfh[i] = XCBGetTransientForHintCookie(_wm.dpy, wins[i]);
                managerequest(wins[i], &managecookies[i * ManageCookieLAST]);
            }
            
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
                {   managereply(wins[i], &managecookies[i * ManageCookieLAST]);
                }
                else if(replystates[i] && replystates[i]->map_state == XCB_WINDOW_ICONIC_STATE)
                {   managereply(wins[i], &managecookies[i * ManageCookieLAST]);
                }
            }

            /* now the transients */
            for(i = 0; i < num; ++i)
            {   
                if(trans[i])
                {
                    if(replies[i]->map_state == XCB_MAP_STATE_VIEWABLE && replystates[i] && replystates[i]->map_state == XCB_WINDOW_ICONIC_STATE)
                    {
                        /* technically we shouldnt have to do this but just in case */
                        if(!wintoclient(wins[i]))
                        {   managereply(wins[i], &managecookies[i * ManageCookieLAST]);
                        }
                    }
                }
            }
            /* cleanup */
            for(i = 0; i < num; ++i)
            {
                free(replies[i]);
                free(replystates[i]);
            }
            free(wa);
            free(wastates);
            free(tfh);
            free(managecookies);
            free(replies);
            free(replystates);
            free(trans);
        }
        free(tree);
    }
    else
    {   DEBUG0("Failed to scan for clients.");
    }
    /* restore session covers this after */
    /* arrangemons(); */
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
    setclientdesktop(c, m->desksel);
    focus(NULL);
    /* arrangeall() */
}

void 
setdesktopcount(Monitor *m, uint16_t desktops)
{
    const u8 MIN_DESKTOPS = 1;
    if(desktops <= MIN_DESKTOPS)
    {   DEBUG0("Cannot make desktop count less than possible.");
        return;
    }
    if(m->deskcount == desktops)
    {
        DEBUG("Desktops are already at specified capacity: [%u]", desktops);
        return;
    }
    u16 i;
    Desktop *desk = m->desklast;
    if(m->deskcount > desktops)
    {
        Client *c;
        Desktop *tmp = NULL;
        for(i = desktops; i > m->deskcount; --i)
        {
            if(desk && prevdesktop(desk))
            {   
                for(c = desk->clients; c; c = nextclient(c))
                {   
                    setclientdesktop(c, prevdesktop(desk));
                } 
                tmp = prevdesktop(desk);
                detachdesktop(m, desk);
                cleanupdesktop(desk);
            }
            desk = tmp;
        }
    }
    else
    {
        u8 failurecount = 0;
        for(i = 0; i < desktops && failurecount < 10; ++i)
        {
            desk = createdesktop();
            if(desk)
            {   attachdesktoplast(m, desk);
            }
            else
            {   
                i--;
                ++failurecount;
            }
        }
        if(failurecount)
        {   DEBUG("Failed [%d]", failurecount);
        }
    }
}

void
setdesktoplayout(Desktop *desk, uint8_t layout)
{
    desk->olayout = desk->layout;
    desk->layout = layout;
}

void 
setdesktopsel(Monitor *mon, Desktop *desksel)
{
    if(desksel->mon != mon)
    {   /* TODO maybe add functionality to detach desktop or something? */
        DEBUG0("Cant set desktop of different monitor, FIXME");
        return;
    }
    if(mon->desksel != desksel)
    {
        mon->desksel = desksel;
        Desktop *desk;
        Client *c;
        int samedesk = 0;
        for(desk = mon->desktops; desk; desk = nextdesktop(desk))
        {
            samedesk = desk == desksel;
            for(c = desk->stack; c; c = nextstack(c))
            {   showhide(c, samedesk || ISSTICKY(c));
            }
        }
        updatedesktop();
    }
    else
    {   DEBUG0("Same desktop, no change.");
    }
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
    /* Most java apps require this see:
     * https://wiki.archlinux.org/title/Java#Impersonate_another_window_manager
     * https://wiki.archlinux.org/title/Java#Gray_window,_applications_not_resizing_with_WM,_menus_immediately_closing
     * for more information.
     * "Hard coded" window managers to ignore "Write Once, Debug Everywhere"
     * This fixes java apps just having a blank white screen on some screen instances.
     * One example is Ghidra, made by the CIA.
     */
    _wm.wmname = "LG3D";

    if(!_wm.syms)
    {   
        cleanup();
        DIECAT("%s", "Could not establish connection with keyboard (OutOfMemory)");
    }
    /* finds any monitor's */
    updategeom();

    setupatoms();
    setupcursors();
    setupcfg();
    /* supporting window for NetWMCheck */
    _wm.wmcheckwin = XCBCreateSimpleWindow(_wm.dpy, _wm.root, 0, 0, 1, 1, 0, 0, 0);
    XCBSelectInput(_wm.dpy, _wm.wmcheckwin, XCB_NONE);
    XCBChangeProperty(_wm.dpy, _wm.wmcheckwin, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm.wmcheckwin, 1);
    XCBChangeProperty(_wm.dpy, _wm.wmcheckwin, netatom[NetWMName], netatom[NetUtf8String], 8, XCB_PROP_MODE_REPLACE, _wm.wmname, strlen(_wm.wmname) + 1);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm.wmcheckwin, 1);
    /* EWMH support per view */
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom, NetLast);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&wmatom, WMLast);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&gtkatom, GTKLAST);
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&motifatom, 1);

    XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetClientList]);
    
    updatedesktopnum();
    updatedesktop();
    updatedesktopnames();
    updateviewport();

    XCBWindowAttributes wa;
    /* xcb_event_mask_t */
    /* ~0 causes event errors because some event masks override others, for some reason... */
    wa.cursor = cursors[CurNormal];
    wa.event_mask =  XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                    |XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_BUTTON_PRESS
                    |XCB_EVENT_MASK_BUTTON_RELEASE
                    |XCB_EVENT_MASK_POINTER_MOTION
                    |XCB_EVENT_MASK_ENTER_WINDOW
                    |XCB_EVENT_MASK_LEAVE_WINDOW
                    |XCB_EVENT_MASK_STRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_PROPERTY_CHANGE
                    ;   /* the ; is here just so its out of the way */
    XCBChangeWindowAttributes(_wm.dpy, _wm.root, XCB_CW_EVENT_MASK|XCB_CW_CURSOR, &wa);
    XCBSelectInput(_wm.dpy, _wm.root, wa.event_mask);
    /* init numlock */
    updatenumlockmask();
    grabkeys();
    focus(NULL);
}

void
setupatoms(void)
{
    XCBCookie motifcookie;
    XCBCookie wmcookie[WMLast];
    XCBCookie netcookie[NetLast];
    XCBCookie gtkcookie[GTKLAST];

    motifcookie = XCBInternAtomCookie(_wm.dpy, "_MOTIF_WM_HINTS", False);
    XCBInitWMAtomsCookie(_wm.dpy, (XCBCookie *)wmcookie);
    XCBInitNetWMAtomsCookie(_wm.dpy, (XCBCookie *)netcookie);
    XCBInitGTKAtomsCookie(_wm.dpy, (XCBCookie *)gtkcookie);

    /* replies */
    XCBInitWMAtomsReply(_wm.dpy, wmcookie, wmatom);
    XCBInitNetWMAtomsReply(_wm.dpy, netcookie, netatom);
    XCBInitGTKAtomsReply(_wm.dpy, gtkcookie, gtkatom);
    motifatom = XCBInternAtomReply(_wm.dpy, motifcookie);
}

void
setupbar(Monitor *m, Client *bar)
{
    detachcompletely(bar);
    configure(bar);
    m->bar = bar;
    setoverrideredirect(bar, 1);
    setborderwidth(bar, 0);
    setdisableborder(bar, 1);
    setfullscreen(bar, 0);
    sethidden(bar, 0);
    setsticky(bar, 1);
    updatebargeom(m);
    updatebarpos(m);
    DEBUG("Found a bar: [%d]", bar->win);
}

void
setupcursors(void)
{
    cursors[CurNormal] = XCBCreateFontCursor(_wm.dpy, XC_left_ptr);
    cursors[CurResizeTopL] = XCBCreateFontCursor(_wm.dpy, XC_top_left_corner);
    cursors[CurResizeTopR] = XCBCreateFontCursor(_wm.dpy, XC_top_right_corner);
    cursors[CurMove] = XCBCreateFontCursor(_wm.dpy, XC_fleur);
}

void
setupcfg(void)
{
    /* TODO */
    setupcfgdefaults();
}

void
setupcfgdefaults(void)
{
    /* Do note these settings are mostly arbitrary numbers that I (the creator) like */
    UserSettings *s = &_cfg;
    const u16 nmaster = 1;
    const u8 hoverfocus = 0;   /* bool */
    const u8 desktoplayout = Monocle;
    const u8 odesktoplayout = Tiled;
    const u8 defaultdesktop = 0;
    const u16 refreshrate = 60;
    const float bgw = 0.95f;
    const u16 winsnap = 10;
    const u16 maxcc = 256;
    const float mfact = 0.55f;

    USSetMCount(s, nmaster);
    USSetLayout(s, desktoplayout);
    USSetOLayout(s, odesktoplayout);
    USSetDefaultDesk(s, defaultdesktop);
    USSetHoverFocus(s, hoverfocus);
    USSetRefreshRate(s, refreshrate);
    USSetGapRatio(s, bgw);
    USSetSnap(s, winsnap);
    USSetMaxClientCount(s, maxcc);
    USSetMFact(s, mfact);

    BarSettings *bs = USGetBarSettings(&_cfg);
    /* Left Stuff */
    bs->left.w = .15f;
    bs->left.h = 1.0f;
    bs->left.x = 0.0f;
    bs->left.y = 0.0f;
    /* Right Stuff */
    bs->right.w = .15f;
    bs->right.h = 1.0f;
    bs->right.x = 1.0f - bs->right.w;
    bs->right.y = 0.0f;
    /* Top Stuff */
    bs->top.w = 1.0f;
    bs->top.h = .15f;
    bs->top.x = 0.0f;
    bs->top.y = 0.0f;
    /* Bottom Stuff */
    bs->bottom.w = 1.0f;
    bs->bottom.h = .15f;
    bs->bottom.x = 0.0f;
    bs->bottom.y = 1.0f - bs->bottom.h;
}

void
updatedesktop(void)
{
    u32 data = _wm.selmon->desksel->num;
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetCurrentDesktop], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&data, 1);
}

void
updatedesktopnames(void)
{
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopNames], XCB_ATOM_STRING, 8, XCB_PROP_MODE_REPLACE, "~0", _wm.selmon->deskcount);
}

void
updatedesktopnum(void)
{
    i32 data =  _wm.selmon->deskcount;
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetNumberOfDesktops], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&data, 1);
}

void
updateviewport(void)
{
    i32 data[2] = { 0, 0 };
    XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetDesktopViewport], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
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
            err =   "Could Hold connection to the XServer for whatever reason BadConnection Error.";
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

/* how to return.
 * reference point is c1.
 *
 * sort order is 1,2,3,4,5,6
 */
static int
__stack_priority_helper_above(unsigned int x1, unsigned int x2)
{
    return x1 < x2;
}
/* how to return.
 * reference point is c1.
 *
 * sort order is 6,5,4,3,2,1
 */
static int 
__stack_priority_helper_below(unsigned int x1, unsigned int x2)
{
    return x1 > x2;
}

int
stackpriority(Client *c1, Client *c2)
{
    const unsigned int dock1 = ISDOCK(c1);
    const unsigned int dock2 = ISDOCK(c2);

    const unsigned int above1 = ISALWAYSONTOP(c1);
    const unsigned int above2 = ISALWAYSONTOP(c2);

    const unsigned int float1 = ISFAKEFLOATING(c1);
    const unsigned int float2 = ISFAKEFLOATING(c2);

    const unsigned int below1 = ISBELOW(c1);
    const unsigned int below2 = ISBELOW(c2);

    const unsigned int hidden1 = ISHIDDEN(c1);
    const unsigned int hidden2 = ISHIDDEN(c2);

    /* Bottom Restacking */
    if(below1 ^ below2)
    {   
        DEBUG0("BELOW");
        return __stack_priority_helper_below(below1, below2);
    }
    else if(hidden1 ^ hidden2)
    {   
        DEBUG0("HIDDEN");
        return __stack_priority_helper_below(hidden1, hidden2);
    }
    /* Regular restacking */
    else if(dock1 ^ dock2)
    {   
        DEBUG0("DOCK");
        return __stack_priority_helper_above(dock1, dock2);
    }
    else if(above1 ^ above2)
    {   
        DEBUG0("ABOVE");
        return __stack_priority_helper_above(above1, above2);
    }
    else if(float1 ^ float2)
    {   
        DEBUG0("FLOAT");
        return __stack_priority_helper_above(float1, float2);
    }

    DEBUG0("FOCUS");
    /* focus is forward order so, we must calculate reversely */
    return __stack_priority_helper_above(c2->rstacknum, c1->rstacknum);
}

void
startup(void)
{
#ifdef __OpenBSD__
        if (pledge("stdio rpath proc exec", NULL) == -1)
            die("pledge");
#endif /* __OpenBSD__ */
    if(!setlocale(LC_CTYPE, ""))
    {   fputs("WARN: NO_LOCALE_SUPPORT\n", stderr);
    }
    char *display = NULL;
    _wm.dpy = XCBOpenDisplay(display, &_wm.screen);
    display = display ? display : getenv("DISPLAY");
    DEBUG("DISPLAY -> %s", display);
    if(!_wm.dpy)
    {   DIECAT("FATAL: Cannot Connect to X Server. [%s]", display);
    }
    checkotherwm();
    /* This allows for execvp and exec to only spawn process on the specified display rather than the default varaibles */
    if(display)
    {   setenv("DISPLAY", display, 1);
    }
    /* keybinds hash */
    atexit(exithandler);
#ifndef DEBUG
    XCBSetErrorHandler(xerror);
#endif
}

void
tile(Desktop *desk)
{
    const u16 nmaster = USGetMCount(&_cfg);
    const float mfact = USGetMFact(&_cfg);
    const float bgwr = USGetGapRatio(&_cfg);

    i32 h, mw, my, ty;
    i32 n, i;
    i32 nx, ny;
    i32 nw, nh;

    Client *c = NULL;
    Monitor *m = desk->mon;

    n = 0;
    for(c = desk->stack; c; c = nextstack(c))
    {   n += !ISFLOATING(c);
    }

    if(!n) 
    {   return;
    }
    
    if(n > nmaster)
    {   mw = nmaster ? m->ww * mfact: 0;
    }
    else
    {   mw = m->ww;
    }

    i = my = ty = 0;
    for (c = desk->stack; c; c = nextstack(c))
    {
        if(ISFLOATING(c))
        {   continue;
        }
        if (i < nmaster)
        {
            h = (m->wh - my) / (MIN(n, nmaster) - i);
            nx = m->wx;
            ny = m->wy + my;
            nw = mw - c->bw * 2;
            nh = h - c->bw * 2;

            nx += (nw - (nw * bgwr)) / 2;
            ny += (nh - (nh * bgwr)) / 2;
            nw *= bgwr;
            nh *= bgwr;

            resize(c, nx, ny, nw, nh, 0);
            if (my + HEIGHT(c) < m->wh) 
            {   my += HEIGHT(c);
            }
        }
        else
        {
            h = (m->wh - ty) / (n - i);
            nx = m->wx + mw;
            ny = m->wy + ty;
            nw = m->ww - mw - c->bw * 2;
            nh = h - c->bw * 2;

            nx += (nw - (nw * bgwr)) / 2;
            ny += (nh - (nh * bgwr)) / 2;
            nw *= bgwr;
            nh *= bgwr;

            resize(c, nx, ny, nw, nh, 0);
            if (ty + HEIGHT(c) < m->wh) 
            {   ty += HEIGHT(c);
            }
        }
        ++i;
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
        unique = calloc(nn, sizeof(xcb_xinerama_query_screens_reply_t));
        if(!unique)
        {   return dirty;
        }
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
                detachfocus(c);
                c->desktop = _wm.mons->desktops;
                /* TODO desktops might break. */
				attach(c);
				attachstack(c);
                attachfocus(c);
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
updatebarpos(Monitor *m)
{
    /* reset space */
    m->ww = m->mw;
    m->wh = m->mh;
    m->wx = m->mx;
    m->wy = m->my;
    Client *bar = m->bar;
    if(!bar)
    {   return;
    }
    enum BarSides side = GETBARSIDE(m, bar, 0);
    if(ISFIXED(bar))
    {
        if(bar->w > bar->h)
        {   
            /* is it top bar ? */
            if(bar->y + bar->h / 2 <= m->my + m->mh / 2)
            {   side = BarSideTop;
            }
            /* its bottom bar */
            else
            {   side = BarSideBottom;
            }
        }
        else if(bar->w < bar->h)
        {
            /* is it left bar? */
            if(bar->x + bar->w / 2 <= m->mx + m->mw / 2)
            {   side = BarSideLeft;
            }
            /* its right bar */
            else
            {   side = BarSideRight;
            }
        }
        else
        {   DEBUG0("Detected bar is a square suprisingly.");
        }
    }
    if(!ISHIDDEN(bar))
    {
        switch(side)
        {
            case BarSideLeft:
                /* make sure its on the left side */
                resize(bar, m->mx, m->my, bar->w, bar->h, 1);
                m->wx += bar->w;
                m->ww -= bar->w;
                DEBUG0("Bar Placed Left.");
                break;
            case BarSideRight:
                /* make sure its on the right side */
                resize(bar, m->mx + (m->mw - bar->w), m->my, bar->w, bar->h, 1);
                m->ww -= bar->w;
                DEBUG0("Bar Placed Right.");
                break;
            case BarSideTop:
                /* make sure its on the top side */
                resize(bar, m->mx, m->my, bar->w, bar->h, 1);
                m->wy += bar->h;
                m->wh -= bar->h;
                DEBUG0("Bar Placed Top.");
                break;
            case BarSideBottom:
                /* make sure its on the bottom side */
                resize(bar, m->mx, m->my + (m->mh - bar->h), bar->w, bar->h, 1);
                m->wh -= bar->h;
                DEBUG0("Bar Placed Bottom.");
                break;
            default:
                break;
        }
    }
    else
    {   
        switch(side)
        {
            case BarSideLeft:
                bar->x = -bar->w;
                break;
            case BarSideRight:
                bar->x = m->mw + bar->w;
                break;
            case BarSideTop:
                bar->y = -bar->h;
                break;
            case BarSideBottom:
                bar->y = m->mh + bar->h;
                break;
            default:
                /* just warp offscreen */
                bar->x = m->mw;
                bar->y = m->mh;
                break;
        }
    }
    resize(bar, bar->x, bar->y, bar->w, bar->h, 1);
}

void
updatebargeom(Monitor *m)
{
    Client *bar = m->bar;
    if(!bar || ISHIDDEN(bar))
    {   return;
    }
    /* if the bar is fixed then the geom is impossible to update, also we dont want to update our current bar status cause of that also */
    if(ISFIXED(bar))
    {   return;
    }
    BarSettings *bs = USGetBarSettings(&_cfg);

    f32 bxr;
    f32 byr;
    f32 bwr;
    f32 bhr;
    enum BarSides side = GETBARSIDE(m, bar, 0);
    enum BarSides prev = GETBARSIDE(m, bar, 1);
    if(prev != side)
    {
        switch(side)
        {   
            case BarSideLeft:
                bxr = bs->left.x;
                byr = bs->left.y;
                bwr = bs->left.w;
                bhr = bs->left.h;
                resize(bar, m->mx + (m->mw * bxr), m->my + (m->mh * byr), m->mw * bwr, m->mh * bhr, 1);
                break;
            case BarSideRight:
                bxr = bs->right.x;
                byr = bs->right.y;
                bwr = bs->right.w;
                bhr = bs->right.h;
                resize(bar, m->mx + (m->mw * bxr), m->my + (m->mh * byr), m->mw * bwr, m->mh * bhr, 1);
                break;
            case BarSideTop:
                bxr = bs->top.x;
                byr = bs->top.y;
                bwr = bs->top.w;
                bhr = bs->top.h;
                resize(bar, m->mx + (m->mw * bxr), m->my + (m->mh * byr), m->mw * bwr, m->mh * bhr, 1);
                break;
            case BarSideBottom:
                bxr = bs->bottom.x;
                byr = bs->bottom.y;
                bwr = bs->bottom.w;
                bhr = bs->bottom.h;
                resize(bar, m->mx + (m->mw * bxr), m->my + (m->mh * byr), m->mw * bwr, m->mh * bhr, 1);
                break;
        }
    }
    else
    {
        switch(side)
        {
            case BarSideLeft:
                bs->left.x = bar->x;
                bs->left.y = bar->y;
                bs->left.w = bar->w;
                bs->left.h = bar->h;
                break;
            case BarSideRight:
                bs->right.x = bar->x;
                bs->right.y = bar->y;
                bs->right.w = bar->w;
                bs->right.h = bar->h;
                break;
            case BarSideTop:
                bs->top.x = bar->x;
                bs->top.y = bar->y;
                bs->top.w = bar->w;
                bs->top.h = bar->h;
                break;
            case BarSideBottom:
                bs->bottom.x = bar->x;
                bs->bottom.y = bar->y;
                bs->bottom.w = bar->w;
                bs->bottom.h = bar->h;
                break;
        }
    }
}

void
updateclientlist(XCBWindow win, uint8_t type)
{
    Monitor *m;
    Desktop *desk;
    Client *c;
    switch(type)
    {
        case ClientListAdd:
            XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&(win), 1);
            break;
        case ClientListRemove: case ClientListReload:
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
            break;
        default:
            DEBUG0("No type specified.");
            break;
    }
}

/* this function is really slow, slower than malloc use only in startup or rare mapping changes */
void
updatenumlockmask(void)
{
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
updatestackpriorityfocus(Desktop *desk)
{
    Client *c;
    int i = 0;
    for(c = desk->focus; c; c = nextfocus(c))
    {
        c->rstacknum = ++i;
        if(ISFLOATING(c) && DOCKED(c))
        {   setfloating(c, 0);
        }
    }
}

void
wakeupconnection(XCBDisplay *display, int screen)
{
    if(!display)
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
    XCBSendEvent(display, XCBRootWindow(display, screen), False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
    /* make sure display gets the event (duh) */
    XCBFlush(_wm.dpy);
}


Monitor *
wintomon(XCBWindow win)
{
    i16 x, y;
    Client *c;
    Monitor *m;
    if(win == _wm.root && getrootptr(&x, &y)) 
    {   return recttomon(x, y, 1, 1);
    }
    for (m = _wm.mons; m; m = m->next)
    {   
        if (win == m->bar->win) 
        {   return m;
        }
    }
    if ((c = wintoclient(win))) 
    {   return c->desktop->mon;
    }
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
#ifdef ENABLE_DEBUG
        XCBCookie id;
        id.sequence = err->sequence;
        DEBUG("%s()", XCBDebugGetNameFromId(id));
#endif
    }
}

int
main(int argc, char *argv[])
{
    argcvhandler(argc, argv);
    startup();
    setup();
    scan();
    restoresession();
    run();
    cleanup();
    specialconds(argc, argv);
    return EXIT_SUCCESS;
}

/* See LICENSE file for copyright and license details.
 * 4 Tab spaces; No tab characters use spaces for tabs
 * Basic overview of dwm => https://ratfactor.com/dwm
 * For more information about xlib (X11)       visit https://x.org/releases/current/doc/libX11/libX11/libX11.html
 * For a quick peak at commonly used functions visit https://tronche.com/gui/x/xlib/
 * Cursors : https://tronche.com/gui/x/xlib/appendix/b/
 * XCursor:  https://man.archlinux.org/man/Xcursor.3
 * EWMH:     https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html
 * XEvent:   https://tronche.com/gui/x/xlib/events/structures.html
 * Document: https://www.x.org/releases/X11R7.5/doc/x11proto/proto.pdf
 */


/* DEBUGGING
 * Stuff you need gdb xephyr
 * sudo pacman -S gdb xorg-server-xephyr
 *
 *
 * first make sure its compiled in DEBUG using config.mk
 *
 * run this command: Xephyr :1 -ac -resizeable -screen 680x480 &
 * set the display to the one you did for Xephyr in this case we did 1 so
 * run this command: export DISPLAY=:1
 * now you are mostly done
 * run this command: gdb dwm
 * you get menu
 * run this command: lay spl
 * you get layout and stuff
 * 
 * now basic gdb stuff
 * break or b somefunction # this sets a break point for whatever function AKA stop the code from running till we say so
 * next or n # this moves to the next line of logic code (logic code is current code line)
 * step or s # this moves to the next line of code (code being actual code so functions no longer exist instead we just go there)
 * ctrl-l # this resets the window thing which can break sometimes (not sure why it hasnt been fixed but ok)
 * skip somefunction # this tries to skip a function if it can but ussualy is worthless (AKA I dont know how to use it)(skip being not show to you but it does run in the code)
 *
 * after your done
 * run this command: exit
 * 
 */
