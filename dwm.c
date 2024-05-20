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

#include <X11/cursorfont.h>     /* cursors */

#include <X11/X.h> /* error codes */


/* keycodes */
#include <X11/keysym.h>

#include "util.h"
#include "dwm.h"
#include "parser.h"

#include "keybinds.h"



/* for HELP/DEBUGGING see under main() or the bottom */

extern void (*handler[]) (XCBGenericEvent *);


WM _wm;
UserSettings _cfg;

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];
XCBAtom motifatom;
XCBCursor cursors[CurLast];

/* Macro definitions */
u16 OLDWIDTH(Client *c)         { return (c->oldw + (c->bw * 2)); }
u16 OLDHEIGHT(Client *c)        { return (c->oldw + (c->bw * 2)); }
u16 WIDTH(Client *c)            { return (c->w + (c->bw * 2)); }
u16 HEIGHT(Client *c)           { return (c->h + (c->bw * 2)); } 
/* Our custom states */
int ISALWAYSONTOP(Client *c)    { return c->wstateflags & _STATE_ABOVE; }
int ISALWAYSONBOTTOM(Client *c) { return c->wstateflags & _STATE_BELOW; }
int WASFLOATING(Client *c)      { return c->flags & _FSTATE_WASFLOATING; }
int ISFLOATING(Client *c)       { return c->flags & _FSTATE_FLOATING; }
int WASDOCKED(Client *c)   {   
                                    const i16 wx = c->desktop->mon->wx;
                                    const i16 wy = c->desktop->mon->wy;
                                    const u16 ww = c->desktop->mon->ww;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 x = c->oldx;
                                    const i16 y = c->oldy;
                                    const u16 w = OLDWIDTH(c);
                                    const u16 h = OLDHEIGHT(c);
                                    return !((wx != x) + (wy != y) + (ww != w) + (wh != h));
                                }
int DOCKED(Client *c)       {
                                    const i16 wx = c->desktop->mon->wx;
                                    const i16 wy = c->desktop->mon->wy;
                                    const u16 ww = c->desktop->mon->ww;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 x = c->x;
                                    const i16 y = c->y;
                                    const u16 w = WIDTH(c);
                                    const u16 h = HEIGHT(c);
                                    return !((wx != x) + (wy != y) + (ww != w) + (wh != h));
                                }
int ISFIXED(Client *c)          { return (c->minw != 0) && (c->minh != 0) && (c->minw == c->maxw) && (c->minh == c->maxh); }
int ISURGENT(Client *c)         { return c->wstateflags & _STATE_DEMANDS_ATTENTION; }
int NEVERFOCUS(Client *c)       { return c->wtypeflags & _TYPE_NEVERFOCUS; }
int ISMAXHORZ(Client *c)        { return WIDTH(c) == c->desktop->mon->wh; }
int ISMAXVERT(Client *c)        { return HEIGHT(c) == c->desktop->mon->wh; }
int ISVISIBLE(Client *c)        { return (!!(c->desktop->mon->desksel == c->desktop) || (!!ISSTICKY(c))) && !ISHIDDEN(c); }
int SHOWDECOR(Client *c)        { return c->flags & _FSTATE_SHOW_DECOR; }
int ISSELECTED(Client *c)       { return c->desktop->sel == c; }
        
int COULDBEBAR(Client *c, uint8_t strut) 
                                {
                                    const u8 sticky = !!ISSTICKY(c);
                                    const u8 isdock = !!(ISDOCK(c) | ISTOOLBAR(c));
                                    const u8 above = !!ISABOVE(c); 
                                    return (sticky && strut && (above || isdock));
                                }
/* EWMH Window types */
int ISDESKTOP(Client *c)        { return c->wtypeflags & _TYPE_DESKTOP; }
int ISDOCK(Client *c)           { return c->wtypeflags & _TYPE_DOCK; }
int ISTOOLBAR(Client *c)        { return c->wtypeflags & _TYPE_TOOLBAR; }
int ISMENU(Client *c)           { return c->wtypeflags & _TYPE_MENU; }
int ISUTILITY(Client *c)        { return c->wtypeflags & _TYPE_UTILITY; }
int ISSPLASH(Client *c)         { return c->wtypeflags & _TYPE_SPLASH; }
int ISDIALOG(Client *c)         { return c->wtypeflags & _TYPE_DIALOG; }
int ISDROPDOWNMENU(Client *c)   { return c->wtypeflags & _TYPE_DROPDOWN_MENU; }
int ISPOPUPMENU(Client *c)      { return c->wtypeflags & _TYPE_POPUP_MENU; }
int ISTOOLTIP(Client *c)        { return c->wtypeflags & _TYPE_TOOLTIP; }
int ISNOTIFICATION(Client *c)   { return c->wtypeflags & _TYPE_NOTIFICATION; }
int ISCOMBO(Client *c)          { return c->wtypeflags & _TYPE_COMBO; }
int ISDND(Client *c)            { return c->wtypeflags & _TYPE_DND; }
int ISNORMAL(Client *c)         { return c->wtypeflags & _TYPE_NORMAL; }
int ISMAPICONIC(Client *c)      { return c->wtypeflags & _TYPE_MAP_ICONIC; }
int ISMAPNORMAL(Client *c)                { return !ISMAPICONIC(c); }
/* EWMH Window states */
int ISMODAL(Client *c)          { return c->wstateflags & _STATE_MODAL; }
int ISSTICKY(Client *c)         { return c->wstateflags & _STATE_STICKY; }
int ISMAXIMIZEDVERT(Client *c)  { return c->wstateflags & _STATE_MAXIMIZED_VERT; }
int ISMAXIMIZEDHORZ(Client *c)  { return c->wstateflags & _STATE_MAXIMIZED_HORZ; }
int ISSHADED(Client *c)         { return c->wstateflags & _STATE_SHADED; }
int SKIPTASKBAR(Client *c)      { return c->wstateflags & _STATE_SKIP_TASKBAR; }
int SKIPPAGER(Client *c)        { return c->wstateflags & _STATE_SKIP_PAGER; }
int ISHIDDEN(Client *c)         { return c->wstateflags & _STATE_HIDDEN; }
int ISFULLSCREEN(Client *c)     { return c->wstateflags & _STATE_FULLSCREEN; }
int ISABOVE(Client *c)          { return c->wstateflags & _STATE_ABOVE; }
int ISBELOW(Client *c)          { return c->wstateflags & _STATE_BELOW; }
int DEMANDSATTENTION(Client *c) { return c->wstateflags & _STATE_DEMANDS_ATTENTION; }
int ISFOCUSED(Client *c)        { return c->wstateflags & _STATE_FOCUSED; }
/* WM Protocol */
int HASWMTAKEFOCUS(Client *c)   { return c->wstateflags & _STATE_SUPPORTED_WM_TAKE_FOCUS; }
int HASWMSAVEYOURSELF(Client *c){ return c->wstateflags & _STATE_SUPPORTED_WM_SAVE_YOURSELF; }
int HASWMDELETEWINDOW(Client *c){ return c->wstateflags & _STATE_SUPPORTED_WM_DELETE_WINDOW; }

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
applygravity(const u32 gravity, i16 *x, i16 *y, const u16 w, const u16 h, const u16 bw)
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
    const u16 MAXW = m->mw * 2;
    const u16 MAXH = m->mh * 2;
    /* set minimum possible */
    *width = MAX(1, *width);
    *height = MAX(1, *height);
    /* set max possible */
    *width = MIN(MAXW, *width);
    *height = MIN(MAXH, *height);

    if (interact)
    {
        if (*x > _wm.sw) 
        {   *x = _wm.sw - WIDTH(c);
        }
        if (*y > _wm.sh) 
        {   *y = _wm.sh - HEIGHT(c);
        }
        if (*x + *width + (c->bw * 2) < 0)
        {   *x = 0;
        }
        if (*y + *height + (c->bw * 2) < 0)
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
        if (*x + *width + (c->bw * 2) <= m->wx) 
        {   *x = m->wx;
        }
        if (*y + *height + (c->bw * 2) <= m->wy) 
        {   *y = m->wy;
        }
    }

    if (ISFLOATING(c))
    {
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
arrangeq(Desktop *desk)
{
    Client *c;
    static Desktop *workingdesk = NULL;

    /* skip sending configure request if we are in the same desktop */
    if(workingdesk != desk)
    {
        for(c = desk->stack; c; c = nextstack(c))
        {   /* configuring windows is suprisingly expensive */
            showhide(c);
        }
        workingdesk = desk;
    }
    reorder(desk);
    arrangedesktop(desk);
}

void
arrange(Desktop *desk)
{
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
    /* update the bar or something */
}

/* Macro helper */
#define __attach_helper(STRUCT, HEAD, NEXT, PREV, LAST)     do                                      \
                                                            {   STRUCT->NEXT = STRUCT->HEAD;        \
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
    __attach_helper(c, desktop->stack, snext, sprev, desktop->slast);
}

void
attachrestack(Client *c)
{
    __attach_helper(c, desktop->rstack, rnext, rprev, desktop->rlast);
}

void
attachfocus(Client *c)
{
    __attach_helper(c, desktop->focus, fnext, fprev, desktop->flast);
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
cleanupclient(Client *c)
{
    free(c->wmname);
    free(c->netwmname);
    free(c->classname);
    free(c->instancename);
    free(c->decor);
    free(c->icon);
    free(c);
    c = NULL;
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
clientinitgeom(Client *c, XCBWindowGeometry *wg)
{
    /* Give initial values. */
    c->x = c->oldx = 0;
    c->y = c->oldy = 0;
    c->w = c->oldw = _wm.selmon->ww;
    c->h = c->oldh = _wm.selmon->wh;
    c->bw = 0; /* TODO */

    /* If we got attributes apply them. */
    if(wg)
    {   
        /* init geometry */
        c->x = c->oldx = wg->x;
        c->y = c->oldy = wg->y;
        c->w = c->oldw = wg->width;
        c->h = c->oldh = wg->height;
        c->oldbw = wg->border_width;
        /* if no specified border width default to our own. */
        if(wg->border_width)
        {   c->bw = wg->border_width;
        }
    }
}
void 
clientinitwtype(Client *c, XCBWindowProperty *windowtypereply)
{
    if(windowtypereply)
    {
        XCBAtom *data = XCBGetPropertyValue(windowtypereply);
        const uint32_t ATOM_LENGTH = XCBGetPropertyValueLength(windowtypereply, sizeof(XCBAtom));
        updatewindowtypes(c, data, ATOM_LENGTH);
    }
}

void 
clientinitwstate(Client *c, XCBWindowProperty *windowstatereply)
{
    if(windowstatereply)
    {
        const uint32_t ATOM_LENGTH = XCBGetPropertyValueLength(windowstatereply, sizeof(XCBAtom));
        XCBAtom *data = XCBGetPropertyValue(windowstatereply);
        updatewindowstates(c, data, ATOM_LENGTH);
    }
}

void 
clientinittrans(Client *c, XCBWindow trans)
{
    Client *t;
    if(trans && (t = wintoclient(trans)))
    {   c->desktop = t->desktop;
    }
    else
    {   c->desktop = _wm.selmon->desksel;
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
    /* This uses calloc as we are currently testing stuff, but we will juse malloc and zero it out later in production*/
    Client *c = calloc(1, sizeof(Client ));
    Decoration *decor = createdecoration();
    if(!c || !decor)
    {   
        DEBUG0("Could not allocate memory for client (OutOfMemory).");
        DEBUG("Client:      %p", (void *)c);
        DEBUG("Decoration:  %p", (void *)decor);
        free(c);
        free(decor);
        return NULL;
    }
    c->decor = decor;
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
    c->wmname = NULL;
    c->netwmname = NULL;
    c->classname = NULL;
    c->instancename = NULL;
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
    m->bar = calloc(1, sizeof(Bar ));
    m->__hash = NULL;
    if(!m->bar)
    {   
        DEBUG0("(OutOfMemory) Failed to create bar.");
        cleanupmon(m);
        return NULL;
    }
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
    //DEBUG("%s", XCBGetEventName(cleanev));
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
    {   for(c = desk->focus; c && !ISVISIBLE(c); c = nextfocus(c));
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

        detachfocus(c);
        attachfocus(c);

        grabbuttons(c->win, 1);
        XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);

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
    const u8 MIN_ICON_DATA_SIZE = (MIN_WIDTH + MIN_HEIGHT) * 2;     /* times 2 cause the first and second index are the size */

    u32 *ret = NULL;
    if(iconreply)
    {
        u32 *icon = XCBGetPropertyValue(iconreply);
        size_t size = XCBGetPropertyValueSize(iconreply);
        u32 length = XCBGetPropertyValueLength(iconreply, sizeof(u32));
        if(length >= MIN_ICON_DATA_SIZE)
        {   
            size_t sz = sizeof(u32) * icon[WIDTH_INDEX] * icon[HEIGHT_INDEX] + sizeof(u32) * 2;
            ret = malloc(MIN(sz, size));
            if(ret)
            {   memcpy(ret, icon, MIN(sz, size));
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
grabbuttons(XCBWindow win, uint8_t focused)
{
    u16 i, j;
    /* numlock is int */
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK};
    XCBUngrabButton(_wm.dpy, XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY, win);
    if (!focused)
    {
        XCBGrabButton(_wm.dpy, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY, win, True, BUTTONMASK, 
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE);
    }
    for (i = 0; i < LENGTH(buttons); i++)
    {
        for (j = 0; j < LENGTH(modifiers); j++)
        {
            XCBGrabButton(_wm.dpy, buttons[i].button, 
                    buttons[i].mask | modifiers[j], 
                    win, True, BUTTONMASK, 
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, 
                    XCB_NONE, XCB_NONE);
            //DEBUG("Grabbed button: [%d]", buttons[i].button);
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
    const u16 bgw = 0;

    i32 i, n, cw, ch, aw, ah, cols, rows;
    i32 nx, ny, nw, nh;
    Client *c;
    Monitor *m = desk->mon;

    for(n = 0, c = desk->focus; c; c = nextfocus(c))
    {   
        if(ISFLOATING(c))
        {   continue;
        }
        ++n;
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
    for(i = 0, c = desk->focus; c; c = nextfocus(c))
    {
        if(ISFLOATING(c))
        {   continue;
        }
        nx = m->wx + (i / rows) * cw;
        ny = m->wy + (i % rows) * ch;
        /* adjust height/width of last row/column's windows */
        ah = !!((i + 1) % rows) * (m->wh - ch * rows);
        aw = !!(i >= rows * (cols - 1)) * (m->ww - cw * cols);

        /* _cfg.bgw without fucking everything else */
        nw = cw - (c->bw * 2 + bgw * 2) + aw;
        nh = ch - (c->bw * 2 + bgw * 2) + ah;

        /* Skip resize calls (they are expensive) */
        if(c->x != nx || c->y != ny || c->w != nw || c->h != nh)
        {   resize(c, nx, ny, nw, nh, 0);
        }
        ++i;
    }
}

void 
killclient(Client *c, enum KillType type)
{
    if(!c)
    {   return;
    }
    if(HASWMSAVEYOURSELF(c))
    {   sendprotocolevent(c, wmatom[WMSaveYourself]);
    }
    if(HASWMDELETEWINDOW(c))
    {   sendprotocolevent(c, wmatom[WMDeleteWindow]);
    }
    else
    {
        XCBWindow win = c->win;
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
    unmanage(c, 0);
}

void
managerequest(XCBWindow win, XCBCookie requests[MANAGE_CLIENT_COOKIE_COUNT])
{
    const u32 REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    const u8 STRUT_P_LENGTH = 12;
    const u8 STRUT_LENGTH = 4;
    const u8 NO_BYTE_OFFSET = 0;
    XCBCookie wacookie = XCBGetWindowAttributesCookie(_wm.dpy, win);
    XCBCookie wgcookie = XCBGetWindowGeometryCookie(_wm.dpy, win);
    XCBCookie transcookie = XCBGetTransientForHintCookie(_wm.dpy, win);                        
    XCBCookie wtypecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
    XCBCookie statecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
    XCBCookie sizehcookie = XCBGetWMNormalHintsCookie(_wm.dpy, win);
    XCBCookie wmhcookie = XCBGetWMHintsCookie(_wm.dpy, win);
    XCBCookie classcookie = XCBGetWMClassCookie(_wm.dpy, win);
    XCBCookie wmprotocookie = XCBGetWMProtocolsCookie(_wm.dpy, win, wmatom[WMProtocols]);
    XCBCookie strutpcookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
    XCBCookie strutcookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
    XCBCookie netwmnamecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
    XCBCookie wmnamecookie = XCBGetWindowPropertyCookie(_wm.dpy, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
    XCBCookie pidcookie = XCBGetPidCookie(_wm.dpy, win, netatom[NetWMPid]);
    XCBCookie iconcookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_CARDINAL);

    /* Mostly to prevent needing to rewrite the numbers over and over again if we mess up */
    u8 i = 0;
    requests[i++] = wacookie;
    requests[i++] = wgcookie;
    requests[i++] = transcookie;
    requests[i++] = wtypecookie;
    requests[i++] = statecookie;
    requests[i++] = sizehcookie;
    requests[i++] = wmhcookie;
    requests[i++] = classcookie;
    requests[i++] = wmprotocookie;
    requests[i++] = strutpcookie;
    requests[i++] = strutcookie;
    requests[i++] = netwmnamecookie;
    requests[i++] = wmnamecookie;
    requests[i++] = iconcookie;
    requests[i++] = pidcookie;
    /* 15 elements in thing */
}

Client *
managereply(XCBWindow win, XCBCookie requests[MANAGE_CLIENT_COOKIE_COUNT])
{
    /* checks */
    if(win == _wm.root)
    {   DEBUG("%s", "Cannot manage() root window.");
        return NULL;
    }
    else if(wintoclient(win))
    {   DEBUG("Window already managed????: [%u]", win);
        return NULL;
    } 
    /* barwin checks */
    u8 checkbar = !_wm.selmon->bar->win;
    
    const u16 bw = 0;
    const u32 bcol = 0;
    const u8 showdecor = 1;

    Client *c = NULL;
    XCBWindow trans = 0;
    u8 transstatus = 0;
    const u32 inputmask = XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_FOCUS_CHANGE|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    XCBWindowGeometry *wg = NULL;

    XCBGetWindowAttributes *waattributes = NULL;
    XCBWindowProperty *wtypeunused = NULL;
    XCBWindowProperty *stateunused = NULL;
    XCBSizeHints hints;
    u8 hintstatus = 0;
    XCBWMHints *wmh = NULL;
    XCBWMClass cls = { ._reply = NULL };
    u8 clsstatus = 0;
    XCBWMProtocols wmprotocols = { ._reply = NULL, .atoms_len = 0 };
    u8 wmprotocolsstatus = 0;
    XCBWindowProperty *strutpreply = NULL;
    XCBWindowProperty *strutreply = NULL;
    u32 *strutp = NULL; 
    u32 *strut = NULL;
    XCBWindowProperty *netwmnamereply = NULL;
    XCBWindowProperty *wmnamereply = NULL;
    char *netwmname = NULL;
    char *wmname = NULL;
    XCBWindowProperty *iconreply = NULL;
    pid_t pid = 0;

    /* we do it here before, because we are waiting for replies and for more memory. */
    c = createclient();

    /* wait for replies */
    waattributes = XCBGetWindowAttributesReply(_wm.dpy, requests[0]);
    wg = XCBGetWindowGeometryReply(_wm.dpy, requests[1]);
    transstatus = XCBGetTransientForHintReply(_wm.dpy, requests[2], &trans); trans *= !!transstatus;
    wtypeunused = XCBGetWindowPropertyReply(_wm.dpy, requests[3]);
    stateunused = XCBGetWindowPropertyReply(_wm.dpy, requests[4]);
    hintstatus = XCBGetWMNormalHintsReply(_wm.dpy, requests[5], &hints);
    wmh = XCBGetWMHintsReply(_wm.dpy, requests[6]);
    clsstatus = XCBGetWMClassReply(_wm.dpy, requests[7], &cls);
    wmprotocolsstatus = XCBGetWMProtocolsReply(_wm.dpy, requests[8], &wmprotocols);
    strutpreply = XCBGetWindowPropertyReply(_wm.dpy, requests[9]);
    strutreply = XCBGetWindowPropertyReply(_wm.dpy, requests[10]);
    netwmnamereply = XCBGetWindowPropertyReply(_wm.dpy, requests[11]);
    wmnamereply = XCBGetWindowPropertyReply(_wm.dpy, requests[12]);
    iconreply = XCBGetWindowPropertyReply(_wm.dpy, requests[13]);
    pid = XCBGetPidReply(_wm.dpy, requests[14]);
    strutp = strutpreply ? XCBGetWindowPropertyValue(strutpreply) : NULL;
    strut = strutreply ? XCBGetWindowPropertyValue(strutpreply) : NULL;

    if(!c)
    {   goto FAILURE;
    }
    c->win = win;

    /* On Failure clear flag and ignore hints */
    hints.flags *= !!hintstatus;    

    if(waattributes && waattributes->override_redirect)
    {   DEBUG("Override Redirect: [%d]", win);
        /* theoredically we could manage these but they are a hastle to deal with */
        goto FAILURE;
    }

    clientinitgeom(c, wg);
    clientinitwtype(c, wtypeunused);
    clientinitwstate(c, stateunused);
    clientinittrans(c, trans);
    updatewindowprotocol(c, wmprotocolsstatus ? &wmprotocols : NULL);
    getnamefromreply(netwmnamereply, &netwmname);
    getnamefromreply(wmnamereply, &wmname);

    if(checkbar && COULDBEBAR(c, strutp || strut))
    {
        Bar *bar = managebar(_wm.selmon, win);
        resizebar(bar, c->x, c->y, c->w, c->h);
        free(c);
        c = NULL;
        DEBUG("Found a bar: [%d]", win);
        goto CLEANUP;
    }

    /* Custom stuff */
    setclientpid(c, pid);
    setborderwidth(c, bw);
    setbordercolor32(c, bcol);
    setshowdecor(c, showdecor);
    updatetitle(c, netwmname, wmname);
    updatesizehints(c, &hints);
    if(clsstatus)
    {   updateclass(c, &cls);
    }
    updatewmhints(c, wmh);
    updateicon(c, iconreply);
    XCBSelectInput(_wm.dpy, win, inputmask);
    grabbuttons(win, 0);

    if(!c->desktop)
    {   c->desktop = _wm.selmon->desksel;
    }

    attach(c);
    attachstack(c);
    attachfocus(c);

    updateclientlist(win, ClientListAdd);
    setclientstate(c, XCB_WINDOW_NORMAL_STATE);

    if(c->desktop == _wm.selmon->desksel)
    {   unfocus(_wm.selmon->desksel->sel, 0);
    }

    /* inherit previous client state */
    if(c->desktop && c->desktop->sel)
    {   setfullscreen(c, ISFULLSCREEN(c->desktop->sel) || ISFULLSCREEN(c));
    }
    if(c->desktop)
    {   HASH_ADD_INT(c->desktop->mon->__hash, win, c);
    }
    /* propagates border_width, if size doesn't change */
    configure(c);   
    goto CLEANUP;
FAILURE:
    free(c);
    c = NULL;
    goto CLEANUP;
CLEANUP:
    /* reply cleanup */
    free(waattributes);
    free(wmh);
    free(stateunused);
    free(wtypeunused);
    free(wg);
    XCBWipeGetWMClass(&cls);
    XCBWipeGetWMProtocols(&wmprotocols);
    free(strutpreply);
    free(strutreply);
    /* Dont free we reused mem allocated previously
     * free(netwmnamereply);
     * free(wmnamereply);
     */
    /* maybe no or just memcpy the first icon into a buffer and keep the thing just there. */
    free(iconreply);
    return c;
}

Bar *
managebar(Monitor *m, XCBWindow win)
{
    const u32 inputmask = XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_FOCUS_CHANGE|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    
    if(!win)
    {   return NULL;
    }
    m->bar->win = win;

    DEBUG("New bar: [%d]", win);
    setshowbar(m->bar, 1);
    updatebargeom(_wm.selmon);
    updatebarpos(_wm.selmon);
    XCBSelectInput(_wm.dpy, win, inputmask);
    updateclientlist(win, ClientListAdd);
    XCBMapWindow(_wm.dpy, win);
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
    Client *c;
    Monitor *m = desk->mon;
    i32 nw, nh;
    i32 nx = m->wx;
    i32 ny = m->wy;

    for(c = desk->focus; c; c = nextfocus(c))
    {
        if(ISFLOATING(c))
        {   continue;
        }
        nw = m->ww - (c->bw * 2);
        nh = m->wh - (c->bw * 2);
        if(c->x != nx || c->y != ny || c->w != nw || c->h != nh)
        {   resize(c, nx, ny, nw, nh, 0); 
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
nextrstack(Client *c)
{
    return c ? c->rnext : c;
}

Client *
nextfocus(Client *c)
{
    return c ? c->fnext : c;
}

Client *
nextvisible(Client *c)
{
    for(; c && !ISVISIBLE(c); c = c->next);
    return c;
}

Desktop *
prevdesktop(Desktop *desk)
{
    return desk ? desk->prev : desk;
}

Client *
prevclient(Client *c)
{
    return c ? c->prev : c;
}

Client *
prevfocus(Client *c)
{
    return c ? c->fprev : c;
}

Client *
prevstack(Client *c)
{
    return c ? c->sprev : c;
}

Client *
prevrstack(Client *c)
{   
    return c ? c->rprev : c;
}

Client *
prevvisible(Client *c)
{
    while(c && !ISVISIBLE(c))
    {   c = prevclient(c);
    }
    return c;
}

void
quit(void)
{
    _wm.running = 0;
    wakeupconnection();
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
    arrangemons();
    /* No need to flush run() syncs for us */
    /* XCBFlush(_wm.dpy) */
}

Client *
restoreclientsession(Desktop *desk, char *buff, u16 len)
{
    const u8 SCANF_CHECK_SUM = 14;
    u8 check = 0;

    int x, y;
    int ox, oy;
    unsigned int w, h;
    unsigned int ow, oh;
    XCBWindow WindowId;
    XCBWindow WindowIdFocus;
    XCBWindow WindowIdStack;
    u32 Flags;
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
                    ,
                    &x, &y, &w, &h,
                    &ox, &oy, &ow, &oh,
                    &WindowId,
                    &WindowIdFocus,
                    &WindowIdStack,
                    &BorderWidth,
                    &BorderColor,
                    &Flags
                    );

    if(check == SCANF_CHECK_SUM)
    {
        Client *cclient = wintoclient(WindowId);
        Client *fclient = wintoclient(WindowIdFocus);
        Client *sclient = wintoclient(WindowIdStack);
        if(cclient)
        {
            setborderwidth(cclient, BorderWidth);
            setbordercolor32(cclient, BorderColor);
            resize(cclient, ox, oy, ow, oh, 1);
            resize(cclient, x, y, w, h, 1);
            cclient->flags = Flags;
        }
        if(fclient)
        {
            detachfocus(fclient);
            attachfocus(fclient);
        }
        if(sclient)
        {   detachstack(sclient);
            attachstack(sclient);
        }
        DEBUG("Restored Client: [%d]", cclient ? cclient->win : 0);
        return cclient;
    }
    else
    {   DEBUG("Failed to parse Client str: \"%s\"", buff);
    }
    return NULL;
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
        memset(possible, 0, errorleeway * sizeof(Monitor *));
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
            u8 i;
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
            managebar(pullm, BarId);
            /* TODO */
            setdesktopcount(pullm, DeskCount);
            if(DeskSelNum != pullm->desksel->num)
            {
                Desktop *desk;
                for(desk = pullm->desktops; desk && desk->num != DeskSelNum; desk = nextdesktop(desk));
                if(desk)
                {   setmondesktop(pullm, desk);
                }
            }
        }
        DEBUG("Restored Monitor: [%d]", pullm ? pullm->bar->win : 0);
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
    c->x = x;
    c->y = y;
    c->w = width;
    c->h = height;
    XCBWindowChanges changes =
    {   
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .border_width = c->bw
    };
    XCBConfigureWindow(_wm.dpy, c->win, 
            XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y
            |XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT
            |XCB_CONFIG_WINDOW_BORDER_WIDTH, 
            &changes
            );
    configure(c);
}

static int __cmp(Client *c1, Client *c2);

void
restack(Desktop *desk)
{
    Client *c;
    u8 inrestack = 0;
    u8 config = 0;
    XCBWindowChanges wc;

    wc.stack_mode = XCB_STACK_MODE_BELOW;

    if(desk->mon->bar->win && SHOWBAR(desk->mon->bar))
    {   wc.sibling = desk->mon->bar->win;
    }
    else
    {   /* TODO: Maybe use wc.sibling = _wm.root? That causes error to be generated though. */
        wc.sibling = _wm.wmcheckwin;
    }

    for(c = desk->stack; c; c = nextstack(c))
    {
        /* unattached clients (AKA new clients) have both set to NULL */
        inrestack = c->rprev || c->rnext;
        /* Client holds both lists so we just check if the next's are the same if not configure it */
        config = c->rnext != c->snext || !inrestack;

        if(config)
        {   XCBConfigureWindow(_wm.dpy, c->win, XCB_CONFIG_WINDOW_SIBLING|XCB_CONFIG_WINDOW_STACK_MODE, &wc);
            DEBUG("Configured window: %s", c->netwmname);
        }
        wc.sibling = c->win;
    }
    /* TODO find a better way todo this without causing infinite loop issues FIXME */
    while(desk->rstack)
    {   detachrestack(desk->rstack);
    }
    for(c = desk->slast; c; c = prevstack(c))
    {   attachrestack(c);
    }
}

static i32
__cmp_helper(int32_t x1, int32_t x2)
{
    const u32 bigger = x1 > x2;
    const u32 smaller = !bigger;
    const i32 ret = -bigger + (smaller);
    return ret;
}

static int
__cmp(Client *c1, Client *c2)
{
    /* how to return.
     * reference point is c1.
     * so if c1 has higher priority return 1.
     *
     * RETURN 1 on higher priority
     * RETURN -1 on lesser priority
     * RETURN 0 on lesser priority. 
     */
    /* != basically just skips if they both have it and only success if one of them has it AKA compare */
    if(ISFLOATING(c1) && DOCKED(c1))
    {   setfloating(c1, 0);
    }
    if(ISFLOATING(c2) && DOCKED(c2))
    {   setfloating(c2, 0);
    }

    const u32 dock1 = ISDOCK(c1);
    const u32 dock2 = ISDOCK(c2);

    const u32 above1 = ISALWAYSONTOP(c1);
    const u32 above2 = ISALWAYSONTOP(c2);

    const u32 float1 = ISFLOATING(c1);
    const u32 float2 = ISFLOATING(c2);

    const u32 below1 = ISBELOW(c1);
    const u32 below2 = ISBELOW(c2);

    const u32 hidden1 = ISHIDDEN(c1);
    const u32 hidden2 = ISHIDDEN(c2);

    if(dock1 ^ dock2)
    {   return __cmp_helper(dock1, dock2);
    }
    else if(above1 ^ above2)
    {   return __cmp_helper(above1, above2);
    }
    else if(float1 ^ float2)
    {   return __cmp_helper(float1, float2);
    }
    else if(below1 ^ below2)
    {   return __cmp_helper(below1, below2);
    }
    else if(hidden1 ^ hidden2)
    {   return __cmp_helper(hidden1, hidden2);
    }
    /* till I find a better way this should decrease cpu a bit
     * this takes about 90% of the cycles of compare so find a better way to get newest focus FIXME
     */
    for(; c1 && c1 != c2; c1 = prevfocus(c1));
    return c1 == c2;
}

/*
 * This function is copyright 2001 Simon Tatham.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL SIMON TATHAM BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
void
reorder(Desktop *desk)
{
    MERGE_SORT_LINKED_LIST(Client, __cmp, desk->stack, desk->slast, snext, sprev, 1, 0);
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
saveclientsession(FILE *fw, Client *c, unsigned int iteration)
{
    const char *IDENTIFIER = "Client.";
    Client *c1;
    Client *c2;
    XCBWindow focus;
    XCBWindow stack;
    int i;

    c1 = c->desktop->flast;
    c2 = c->desktop->slast;
    for(i = 0; i < iteration; ++i)
    {   
        c1 = prevfocus(c);
        c2 = prevstack(c);
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
            c->flags
            );
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
    int i = 0;
    for(c = desk->clast; c; c = prevclient(c))
    {   saveclientsession(fw, c, i++);
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
            m->bar->win,
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
            const size_t managecookiesz = sizeof(XCBCookie) * MANAGE_CLIENT_COOKIE_COUNT;
            XCBCookie *wa = malloc(countsz);
            XCBCookie *wastates = malloc(countsz);
            XCBCookie *tfh = malloc(countsz);
            XCBCookie **managecookies = malloc(sizeof(XCBCookie *) * num);
            XCBGetWindowAttributes **replies = malloc(sizeof(XCBGetWindowAttributes *) * num);
            XCBGetWindowAttributes **replystates = malloc(sizeof(XCBGetWindowAttributes *) * num);
            XCBWindow *trans = malloc(sizeof(XCBWindow) * num);

            if(!wa || !wastates || !tfh || !managecookies)
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
                managecookies[i] = malloc(managecookiesz);
                if(managecookies[i])
                {   managerequest(wins[i], managecookies[i]);
                }
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
                if(!managecookies[i])
                {   continue;
                }
                if(replies[i] && replies[i]->map_state == XCB_MAP_STATE_VIEWABLE)
                {   managereply(wins[i], managecookies[i]);
                }
                else if(replystates[i] && replystates[i]->map_state == XCB_WINDOW_ICONIC_STATE)
                {   managereply(wins[i], managecookies[i]);
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
                        {   
                            if(managecookies[i])
                            {   managereply(wins[i], managecookies[i]);
                            }
                        }
                    }
                }
                free(replies[i]);
                free(replystates[i]);
                free(managecookies[i]);
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
    focus(NULL);
    /* restore session covers this after */
    /* arrangemons(); */
}

void
sendprotocolevent(Client *c, XCBAtom proto)
{
    XCBClientMessageEvent ev;
    ev.type = wmatom[WMProtocols];
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = c->win;
    ev.format = 32;
    ev.data.data32[0] = proto;
    ev.data.data32[1] = XCB_CURRENT_TIME;
    XCBSendEvent(_wm.dpy, c->win, False, XCB_NONE, (const char *)&ev);
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
setalwaysontop(Client *c, u8 state)
{
    SETFLAG(c->wstateflags, _STATE_ABOVE, !!state);
}

void
setalwaysonbottom(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_BELOW, !!state);
}

void
setborderalpha(Client *c, uint8_t alpha)
{
    /* remove previous alpha */
    const u32 ccol = c->bcol & ~(UINT8_MAX << 24);
    const u32 col = ccol + (alpha << 24);
    /* TODO */
    setbordercolor32(c, col);
}

void
setbordercolor(Client *c, uint8_t red, uint8_t green, uint8_t blue)
{
    /* get alpha */
    const u32 alpha = c->bcol & (UINT8_MAX << 24);

    const u32 col = blue + (green << 8) + (red << 16) + alpha;
    setbordercolor32(c, col);
}

void
setbordercolor32(Client *c, uint32_t col)
{   
    c->bcol = col;
    XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);
}

void
setborderwidth(Client *c, uint16_t border_width)
{
    c->oldbw = c->bw;
    c->bw = border_width;
    XCBSetWindowBorderWidth(_wm.dpy, c->win, c->bw);
}

void
setclientdesktop(Client *c, Desktop *desk)
{
    detachcompletely(c);
    c->desktop = desk;
    attach(c);
    attachstack(c);
    attachfocus(c);
}

void
setclientstate(Client *c, u8 state)
{
    const i32 data[2] = { state, XCB_NONE };
    XCBChangeProperty(_wm.dpy, c->win, wmatom[WMState], wmatom[WMState], 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
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
                {   setclientdesktop(c, prevdesktop(desk));
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
setclientpid(Client *c, pid_t pid)
{
    c->pid = pid;
}

void
setwtypedesktop(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DESKTOP, !!state);
}

void
setwtypedialog(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_DIALOG, !!state);
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
setwtypeneverfocus(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_NEVERFOCUS, !!state);
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
setwtypemapiconic(Client *c, uint8_t state)
{
    SETFLAG(c->wtypeflags, _TYPE_MAP_ICONIC, !!state);
}

void
setwtypemapnormal(Client *c, uint8_t state)
{
    setwtypemapiconic(c, !state);
}

void 
setwmtakefocus(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SUPPORTED_WM_TAKE_FOCUS, !!state);
}

void 
setwmsaveyourself(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SUPPORTED_WM_SAVE_YOURSELF, !!state);
}

void 
setwmdeletewindow(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SUPPORTED_WM_DELETE_WINDOW, !!state);
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
setshowdecor(Client *c, uint8_t state)
{
    if(state)
    {   
        if(c->decor->win)
        {   XCBMapWindow(_wm.dpy, c->decor->win);   
        }
    }
    else
    {
        if(c->decor->win)
        {   XCBUnmapWindow(_wm.dpy, c->decor->win);
        }
    }
    SETFLAG(c->flags, _FSTATE_SHOW_DECOR, !!state);
}

void
setfullscreen(Client *c, u8 state)
{
    Monitor *m = c->desktop->mon;
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
setfloating(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _FSTATE_WASFLOATING, !!(c->flags & _FSTATE_FLOATING));
    SETFLAG(c->flags, _FSTATE_FLOATING, !!state);
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
    if(HASWMTAKEFOCUS(c))
    {   sendprotocolevent(c, wmatom[WMTakeFocus]);
    }
}

void 
sethidden(Client *c, uint8_t state)
{
    if(state)
    {   winsetstate(c->win, XCB_WINDOW_ICONIC_STATE);
        setwtypemapiconic(c, 1);
    }
    else
    {   winsetstate(c->win, XCB_WINDOW_NORMAL_STATE);
        setwtypemapnormal(c, 1);
    }
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
        {   resize(c, c->x, c->y, c->w, m->wh, 0);
        }
    }
    else
    {
        if(ISMAXVERT(c))
        {   resize(c, c->x, c->y, c->w, c->oldh != m->wh ? c->oldh : c->oldh / 2, 0);
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
        {   resize(c, c->x, c->y, m->ww, c->h, 0);
        }
    }
    else
    {
        if(ISMAXHORZ(c))
        {   resize(c, c->x, c->y, c->oldw != m->ww ? c->oldw : c->oldw / 2, c->h, 0);
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
setmondesktop(Monitor *m, Desktop *desk)
{   
    m->desksel = desk;
}

void
setsticky(Client *c, u8 state)
{
    const XCBWindow win = c->win;
    const XCBAtom replace = !!state * netatom[NetWMStateSticky];
    const u8 length = !!state;
    XCBChangeProperty(_wm.dpy, win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
            XCB_PROP_MODE_REPLACE, (unsigned char *)&replace, length);
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
    updategeom();
    setupcursors();
    setupcfg();
    XCBCookie motifcookie = XCBInternAtomCookie(_wm.dpy, "_MOTIF_WM_HINTS", False);
    XCBInitAtoms(_wm.dpy, wmatom, netatom);
    motifatom = XCBInternAtomReply(_wm.dpy, motifcookie);
    /* supporting window for NetWMCheck */
    _wm.wmcheckwin = XCBCreateSimpleWindow(_wm.dpy, _wm.root, 0, 0, 1, 1, 0, 0, 0);
    XCBSelectInput(_wm.dpy, _wm.wmcheckwin, XCB_NONE);
    XCBChangeProperty(_wm.dpy, _wm.wmcheckwin, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm.wmcheckwin, 1);
    XCBChangeProperty(_wm.dpy, _wm.wmcheckwin, netatom[NetWMName], netatom[NetUtf8String], 8, XCB_PROP_MODE_REPLACE, _wm.wmname, strlen(_wm.wmname) + 1);
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
    wa.cursor = cursors[CurNormal];
    wa.event_mask = 
                    XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE
                    |XCB_EVENT_MASK_POINTER_MOTION|
                    XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_LEAVE_WINDOW
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
    UserSettings *s = &_cfg;
    const u16 nmaster = 1;
    const u8 hoverfocus = 0;   /* bool */
    const u8 desktoplayout = Monocle;
    const u8 odesktoplayout = Tiled;
    const u8 defaultdesktop = 0;
    const u16 refreshrate = 60;
    const u16 bgw = 15;
    const u16 winsnap = 10;
    const u16 maxcc = 256;
    const float mfact = 0.55f;
    const u16 bw = _wm.selmon->ww;
    const u16 bh = _wm.selmon->wh / 10;     /* div 10 is roughly the same as the WSWM old default bar height */
    const i16 bx = _wm.selmon->wx;
    const i16 by = _wm.selmon->wy + bh;     /* because a window is a rectangle calculations start at the top left corner, so we need to add the height */

    USSetMCount(s, nmaster);
    USSetLayout(s, desktoplayout);
    USSetOLayout(s, odesktoplayout);
    USSetDefaultDesk(s, defaultdesktop);
    USSetHoverFocus(s, hoverfocus);
    USSetRefreshRate(s, refreshrate);
    USSetGapWidth(s, bgw);
    USSetSnap(s, winsnap);
    USSetMaxClientCount(s, maxcc);
    USSetMFact(s, mfact);
    USSetBarWidth(s, bw);
    USSetBarHeight(s, bh);
    USSetBarX(s, bx);
    USSetBarY(s, by);
}

void
seturgent(Client *c, uint8_t state) 
{
    XCBCookie wmhcookie = XCBGetWMHintsCookie(_wm.dpy, c->win);
    XCBWMHints *wmh = XCBGetWMHintsReply(_wm.dpy, wmhcookie);
    SETFLAG(c->wstateflags, _STATE_DEMANDS_ATTENTION, !!state);
    if(state)
    {   /* set window border */   
    }
    else
    {   /* set window border */   
    }

    if(wmh)
    {
        wmh->flags = state ? (wmh->flags | XCB_WM_HINT_URGENCY) : (wmh->flags & ~XCB_WM_HINT_URGENCY);
        XCBSetWMHintsCookie(_wm.dpy, c->win, wmh);
        free(wmh);
    }
    /* drawbar */
}

void
updatedecor(Client *c, XCBGetWindowAttributes *wa)
{
    XCBWindow win = c->decor->win;
    return;
    if(!win)
    {   
        XCBCreateWindowValueList values =
        { 
            .override_redirect = 1,
        };
        win = XCBCreateWindow(_wm.dpy, _wm.root, c->x, c->y - 15, c->w, c->h + 15, 0, 
                XCB_COPY_FROM_PARENT, XCB_WINDOW_CLASS_INPUT_ONLY, XCBGetScreen(_wm.dpy)->root_visual, XCB_CW_OVERRIDE_REDIRECT, &values);
        XCBReparentWindow(_wm.dpy, c->win, win, c->x, c->y);
        DEBUG("%u %u", c->win ,win);
        XCBMapWindow(_wm.dpy, win);
        c->decor->win = win;
    }
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
        DEBUG("Not Visible: [%u] -> [%u] vs [%u]", c->win, c->desktop->num, m->desksel->num);
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
    const u16 nmaster = 0;
    const float mfact = 0.0f;
    const u16 bgw = 0;
    
    i32 h = 0, mw = 0, my = 0, ty = 0;
    i32 n = 0, i = 0;
    i32 nx = 0, ny = 0;
    i32 nw = 0, nh = 0;
    Client *c = NULL;
    Monitor *m = NULL;

    m = desk->mon;
    for(n = 0, c = desk->focus; c; c = nextfocus(c))
    {   
        if(ISFLOATING(c))
        {   continue;
        }
        ++n;
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

    for (i = my = ty = 0, c = desk->focus; c; c = nextfocus(c), ++i)
    {
        if(ISFLOATING(c))
        {   --i;
            continue;
        }
        if (i < nmaster)
        {
            h = (m->wh - my) / (MIN(n, nmaster) - i);
            nx = m->wx;
            ny = m->wy + my;
            nw = mw - c->bw * 2;
            nh = h - c->bw * 2;

            /* we divide nw also to get even gaps
             * if we didnt the center gap would be twices as big
             * Although this may be desired, one would simply remove the shift ">>" by 1 in nw 
             */
            nx += bgw;
            ny += bgw;
            nw -= bgw * 2;
            nh -= bgw * 2;
            if(c->x != nx || c->y != ny || c->w != nw || c->h != nh)
            {   resize(c, nx, ny, nw, nh, 0);
            }
            if (my + HEIGHT(c) < (unsigned int)m->wh) 
            {   my += HEIGHT(c) + bgw;
            }
        }
        else
        {
            h = (m->wh - ty) / (n - i);
            nx = m->wx + mw;
            ny = m->wy + ty;
            nw = m->ww - mw - (c->bw << 1);
            nh = h - c->bw * 2;

            nx += bgw / 2;
            ny += bgw;
            nw -= bgw * 2;
            nh -= bgw * 2;
            if(c->x != nx || c->y != ny || c->w != nw || c->h != nh)
            {   resize(c, nx, ny, nw, nh, 0);
            }
            if (ty + HEIGHT(c) < (unsigned int)m->wh) ty += HEIGHT(c) + bgw;
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
updateicon(Client *c, XCBWindowProperty *iconprop)
{
    free(c->icon);
    c->icon = geticonprop(iconprop);
}

void
unmanage(Client *c, uint8_t destroyed)
{
    Desktop *desk = c->desktop;
    if(!c)
    {   return;
    }
    if(!destroyed)
    {   
        /* TODO causes alot of errors for some reason even if its not "destroyed" */
    }
    /* TODO
     * Memory leak if a client is unmaped and maped again
     * (cause we would get the same input focus twice)
     */
    HASH_DEL(desk->mon->__hash, c);
    detachcompletely(c);
    focus(NULL);
    updateclientlist(c->win, ClientListRemove);
    /* no need to arrange fully cause client is not mapped anymore */
    arrange(desk);
    cleanupclient(c);
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
    Bar *bar = m->bar;
    if(SHOWBAR(bar))
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
updateclass(Client *c, XCBWMClass *_class)
{
    const u32 MAX_LEN = 1024;
    if(_class)
    {  
       const u32 CLASS_NAME_LEN = strnlen(_class->class_name, MAX_LEN) + 1;
       const u32 INSTANCE_NAME_LEN = strnlen(_class->instance_name, MAX_LEN) + 1;
       const size_t CLASS_NAME_SIZE = sizeof(char) * CLASS_NAME_LEN;
       const size_t INSTANCE_NAME_SIZE = sizeof(char) * INSTANCE_NAME_LEN;
       char *clsname = malloc(CLASS_NAME_SIZE);
       char *iname = malloc(INSTANCE_NAME_SIZE);

       if(clsname)
       {    
            memcpy(clsname, _class->class_name, CLASS_NAME_SIZE - sizeof(char));
            clsname[CLASS_NAME_LEN - 1] = '\0';
            free(c->classname);
            c->classname = clsname;
       }
       if(iname)
       {
            memcpy(iname, _class->instance_name, INSTANCE_NAME_SIZE - sizeof(char));
            iname[INSTANCE_NAME_LEN - 1] = '\0';
            free(c->instancename);
            c->instancename = iname;
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
updatesizehints(Client *c, XCBSizeHints *size)
{
    /* init values */
    c->basew = c->baseh = 0;
    c->incw = c->inch = 0;
    /* maxw should just be 2 times the max monitor size (if not set) */
    c->maxw = c->desktop->mon->ww * 2;
    c->maxh = c->desktop->mon->wh * 2;
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

    /* if client set max size to none */
    if(!c->maxw)
    {   c->maxw = c->desktop->mon->ww * 2;
    }
    if(!c->maxh)
    {   c->maxh = c->desktop->mon->wh * 2;
    }
}

void
updatetitle(Client *c, char *netwmname, char *wmname)
{
    if(c->wmname != wmname)
    {   free(c->wmname);
        c->netwmname = NULL;
    }
    if(c->netwmname != netwmname)
    {   free(c->netwmname);
        c->netwmname = NULL;
    }
    c->wmname = wmname;
    c->netwmname = netwmname;
}

void
updatewindowprotocol(Client *c, XCBWMProtocols *protocols)
{
    if(protocols && protocols->atoms_len)
    {
        uint32_t i;
        XCBAtom atom;
        for(i = 0; i < protocols->atoms_len; ++i)
        {
            atom = protocols->atoms[i];
            if(atom == wmatom[WMTakeFocus])
            {   setwmtakefocus(c, 1);
            }
            else if(atom == wmatom[WMDeleteWindow])
            {   setwmdeletewindow(c, 1);
            }
            else if(atom == wmatom[WMSaveYourself])
            {   setwmsaveyourself(c, 1);
            }
        }
    }
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
        {   setwtypeneverfocus(c, !wmh->input);
        }
        else
        {   setwtypeneverfocus(c, 0);
        }
        if(wmh->flags & XCB_WM_HINT_STATE)
        {
            switch(wmh->initial_state)
            {   
                case XCB_WINDOW_ICONIC_STATE:
                    sethidden(c, 1);
                    break;
                case XCB_WINDOW_WITHDRAWN_STATE:
                    DEBUG("Window Specified is Widthdrawn? %d", c->win);
                    break;
                case XCB_WINDOW_NORMAL_STATE:
                    break;
                default:
                    break;
            }
        }
        if(wmh->flags & XCB_WM_HINT_ICON_PIXMAP)
        {   /* update icon or something */
        }
        if(wmh->flags & XCB_WM_HINT_ICON_MASK)
        {   /* use flagged bits to asign icon shape */
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
    i32 data[2] = { state, XCB_NONE };
    XCBChangeProperty(_wm.dpy, win, wmatom[WMState], wmatom[WMState], 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}

void *
wintobar(XCBWindow win, uint8_t getmon)
{
    Monitor *m = NULL;
    for(m = _wm.mons; m; m = nextmonitor(m))
    {
        if(m->bar->win == win)
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
    Monitor *m = NULL;

    /* check sel first */
    for(m = _wm.selmon; m; m = nextmonitor(m))
    {   
        HASH_FIND_INT(m->__hash, &win, c);
        if(c)
        {   return c;
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
        if (win == m->bar->win) return m;
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
        XCBCookie id;
        id.sequence = err->sequence;
        DEBUG("%s()", XCBDebugGetNameFromId(id));
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
