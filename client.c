#include <math.h> /* fabsf() */

#include "client.h"

#include "desktop.h"

#include "dwm.h"
#include "keybinds.h"
#include "util.h"

extern WM _wm;
extern XCBAtom netatom[];
extern XCBAtom wmatom[];
extern XCBAtom motifatom;

/* Client struct flags */
#define _FSTATE_FLOATING            ((1 << 0))
#define _FSTATE_WASFLOATING         ((1 << 1))
#define _FSTATE_SHOW_DECOR          ((1 << 2))
#define _FSTATE_OVERRIDE_REDIRECT   ((1 << 3))
#define _FSTATE_KEEP_FOCUS          ((1 << 4))
#define _FSTATE_DISABLE_BORDER      ((1 << 5))
/* EWMH window types */
#define _TYPE_DESKTOP       ((1 << 0))
#define _TYPE_DOCK          ((1 << 1))
#define _TYPE_TOOLBAR       ((1 << 2))
#define _TYPE_MENU          ((1 << 3))
#define _TYPE_UTILITY       ((1 << 4))
#define _TYPE_SPLASH        ((1 << 5))
#define _TYPE_DIALOG        ((1 << 6))
#define _TYPE_DROPDOWN_MENU ((1 << 7))
#define _TYPE_POPUP_MENU    ((1 << 8))
#define _TYPE_TOOLTIP       ((1 << 9))
#define _TYPE_NOTIFICATION  ((1 << 10))
#define _TYPE_COMBO         ((1 << 11))
#define _TYPE_DND           ((1 << 12))
#define _TYPE_NORMAL        ((1 << 13))

/* custom types (using spare bits )*/
#define _TYPE_NEVERFOCUS    ((1 << 14))
/* Window map states, Widthdrawn, Iconic, Normal. */
#define _TYPE_MAP_ICONIC    ((1 << 15))

/* EWMH Window states */
#define _STATE_MODAL                        ((1 << 0))
#define _STATE_STICKY                       ((1 << 1))
#define _STATE_MAXIMIZED_VERT               ((1 << 2))  
#define _STATE_MAXIMIZED_HORZ               ((1 << 3))
#define _STATE_SHADED                       ((1 << 4))
#define _STATE_SKIP_TASKBAR                 ((1 << 5))
#define _STATE_SKIP_PAGER                   ((1 << 6))
#define _STATE_HIDDEN                       ((1 << 7))
#define _STATE_FULLSCREEN                   ((1 << 8))
#define _STATE_ABOVE                        ((1 << 9))
#define _STATE_BELOW                        ((1 << 10))
#define _STATE_DEMANDS_ATTENTION            ((1 << 11))
#define _STATE_FOCUSED                      ((1 << 12))

/* extra states (using spare bits) */
#define _STATE_SUPPORTED_WM_TAKE_FOCUS      ((1 << 13))
#define _STATE_SUPPORTED_WM_SAVE_YOURSELF   ((1 << 14))
#define _STATE_SUPPORTED_WM_DELETE_WINDOW   ((1 << 15))

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
int ISOVERRIDEREDIRECT(Client *c) { return c->flags & _FSTATE_OVERRIDE_REDIRECT; }
int KEEPFOCUS(Client *c)        { return c->flags & _FSTATE_KEEP_FOCUS; }
int DISABLEBORDER(Client *c)    { return c->flags & _FSTATE_DISABLE_BORDER; }
int ISFAKEFLOATING(Client *c)   { return c->flags & _FSTATE_FLOATING || c->desktop->layout == Floating; }

int WASDOCKEDVERT(Client *c)    {   const i16 wy = c->desktop->mon->wy;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 y = c->oldy;
                                    const u16 h = OLDHEIGHT(c);
                                    return (wy == y) && (wh == h);
                                }
int WASDOCKEDHORZ(Client *c)    {   const i16 wx = c->desktop->mon->wx;
                                    const u16 ww = c->desktop->mon->ww;
                                    const i16 x = c->oldx;
                                    const u16 w = OLDWIDTH(c);
                                    return (wx == x) && (ww == w);
                                }

int WASDOCKED(Client *c)        { return WASDOCKEDVERT(c) & WASDOCKEDHORZ(c); }

int DOCKEDVERT(Client *c)       {   const i16 wy = c->desktop->mon->wy;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 y = c->y;
                                    const u16 h = HEIGHT(c);
                                    return (wy == y) && (wh == h);
                                }

int DOCKEDHORZ(Client *c)       {   const i16 wx = c->desktop->mon->wx;
                                    const u16 ww = c->desktop->mon->ww;
                                    const i16 x = c->x;
                                    const u16 w = WIDTH(c);
                                    return (wx == x) && (ww == w);
                                }
int DOCKED(Client *c)           { return DOCKEDVERT(c) & DOCKEDHORZ(c); }


/* used in manage */
int DOCKEDINITIAL(Client *c)    {   Monitor *m = c->desktop->mon;
                                    const i16 wx = m->wx;
                                    const i16 wy = m->my;
                                    const i16 mx = m->mx;
                                    const i16 my = m->my;

                                    const u16 ww = m->ww;
                                    const u16 wh = m->wh;
                                    const u16 mw = m->mw;
                                    const u16 mh = m->mh;

                                    const i16 x = c->x;
                                    const i16 y = c->y;
                                    const u16 w = c->w;
                                    const u16 h = c->h;
                                    const u16 w1 = WIDTH(c);
                                    const u16 h1 = HEIGHT(c);

                                    return
                                        ((wx == x) && (wy == y) && (ww == w) && (wh == h))
                                        ||
                                        ((wx == x) && (wy == y) && (ww == w1) && (wh == h1))
                                        ||
                                        ((mx == x) && (my == y) && (mh == h) && (mw == w))
                                        ||
                                        ((mx == x) && (my == y) && (mh == h1) && (mw == w1))
                                        ;
                                }

int ISFIXED(Client *c)          { return (c->minw != 0) && (c->minh != 0) && (c->minw == c->maxw) && (c->minh == c->maxh); }
int ISURGENT(Client *c)         { return c->wstateflags & _STATE_DEMANDS_ATTENTION; }
int NEVERFOCUS(Client *c)       { return c->wtypeflags & _TYPE_NEVERFOCUS; }
int ISMAXHORZ(Client *c)        { return WIDTH(c) == c->desktop->mon->ww; }
int ISMAXVERT(Client *c)        { return HEIGHT(c) == c->desktop->mon->wh; }
int ISVISIBLE(Client *c)        { return (c->desktop->mon->desksel == c->desktop || ISSTICKY(c)) && !ISHIDDEN(c); }
int SHOWDECOR(Client *c)        { return c->flags & _FSTATE_SHOW_DECOR; }
int ISSELECTED(Client *c)       { return c->desktop->sel == c; }
        
int COULDBEBAR(Client *c, uint8_t strut) 
                                {
                                    const u8 sticky = !!ISSTICKY(c);
                                    const u8 isdock = !!(ISDOCK(c));
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
int ISMAPNORMAL(Client *c)      { return !ISMAPICONIC(c); }
int WTYPENONE(Client *c)        { return c->wtypeflags == 0; }
/* EWMH Window states */
int ISMODAL(Client *c)          { return c->wstateflags & _STATE_MODAL; }
int ISSTICKY(Client *c)         { return c->wstateflags & _STATE_STICKY; }
/* DONT USE */
int ISMAXIMIZEDVERT(Client *c)  { return c->wstateflags & _STATE_MAXIMIZED_VERT; }
/* DONT USE */
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
int WSTATENONE(Client *c)       { return c->wstateflags == 0; }
/* WM Protocol */
int HASWMTAKEFOCUS(Client *c)   { return c->wstateflags & _STATE_SUPPORTED_WM_TAKE_FOCUS; }
int HASWMSAVEYOURSELF(Client *c){ return c->wstateflags & _STATE_SUPPORTED_WM_SAVE_YOURSELF; }
int HASWMDELETEWINDOW(Client *c){ return c->wstateflags & _STATE_SUPPORTED_WM_DELETE_WINDOW; }



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

    if (interact)
    {
        if (*x > _wm.sw) 
        {   *x = _wm.sw - WIDTH(c);
        }
        if (*y > _wm.sh) 
        {   *y = _wm.sh - HEIGHT(c);
        }
        if (*x + *width + (WIDTH(c) - c->w) < 0)
        {   *x = 0;
        }
        if (*y + *height + (HEIGHT(c) - c->h) < 0)
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
        if (*x + *width + (WIDTH(c) - c->w) <= m->wx) 
        {   *x = m->wx;
        }
        if (*y + *height + (HEIGHT(c) - c->h) <= m->wy) 
        {   *y = m->wy;
        }
    }

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
    return *x != c->x || *y != c->y || *width != c->w || *height != c->h;
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
    XCBGenericEvent ev;
    memset(&ev, 0, sizeof(XCBGenericEvent));
    XCBConfigureNotifyEvent *ce = (XCBConfigureNotifyEvent *)&ev;
    ce->response_type = XCB_CONFIGURE_NOTIFY;
    ce->event = _wm.root;
    ce->window = c->win;
    ce->x = c->x;
    ce->y = c->y;
    ce->width = c->w;
    ce->height = c->h;
    ce->border_width = c->bw;
    ce->override_redirect = False;
    XCBSendEvent(_wm.dpy, c->win, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
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

void
focus(Client *c)
{
    Monitor *selmon = _wm.selmon;
    Desktop *desk  = selmon->desksel;
    if(!c || !ISVISIBLE(c))
    {   for(c = desk->focus; c && !ISVISIBLE(c) && !KEEPFOCUS(c); c = nextfocus(c));
    }
    if(desk->sel && desk->sel != c)
    {   unfocus(desk->sel, 0);
    }
    if(c)
    {
        if(c->desktop->mon != _wm.selmon)
        {   _wm.selmon = c->desktop->mon;
        }
        if(c->desktop != _wm.selmon->desksel)
        {   setdesktopsel(_wm.selmon, c->desktop);
        }

        if(ISURGENT(c))
        {   seturgent(c, 0);
        }

        detachfocus(c);
        attachfocus(c);

        grabbuttons(c, 1);
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
grabbuttons(Client *c, uint8_t focused)
{
    /* make sure no other client steals our grab */
    xcb_grab_server(_wm.dpy);
    u16 i, j;
    /* numlock is int */
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK};
    /* Always grab these to allow for replay pointer when focusing by mouse click */
    u8 gbuttons[3] = { LMB, MMB, RMB };

    /* ungrab any previously grabbed buttons that are ours */
    for(i = 0; i < LENGTH(modifiers); ++i)
    {
        for(j = 0; j < LENGTH(gbuttons); ++j)
        {   XCBUngrabButton(_wm.dpy, gbuttons[j], modifiers[i], c->win);
        }
        for(j = 0; j < LENGTH(buttons); ++j)
        {   XCBUngrabButton(_wm.dpy, buttons[j].button, modifiers[i], c->win);
        }
    }
    if (!focused)
    {
        /* grab focus buttons */
        for (i = 0; i < LENGTH(gbuttons); ++i)
        {
            for (j = 0; j < LENGTH(modifiers); ++j)
            {   XCBGrabButton(_wm.dpy, gbuttons[i], modifiers[j], c->win, False, BUTTONMASK, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_SYNC, XCB_NONE, XCB_NONE);
            }
        }
    }
    for (i = 0; i < LENGTH(buttons); ++i)
    {
        for (j = 0; j < LENGTH(modifiers); ++j)
        {
            XCBGrabButton(_wm.dpy, buttons[i].button, 
                    buttons[i].mask | modifiers[j], 
                    c->win, False, BUTTONMASK, 
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC, 
                    XCB_NONE, XCB_NONE);
        }
    }
    xcb_ungrab_server(_wm.dpy);
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
                            _wm.root, True, 
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
killclient(Client *c, enum KillType type)
{
    if(!c)
    {   return;
    }
    if(HASWMSAVEYOURSELF(c))
    {   sendprotocolevent(c, wmatom[WMSaveYourself]);
    }
    if(HASWMDELETEWINDOW(c) && type == Graceful)
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
        XCBGenericEvent ev;
        memset(&ev, 0, sizeof(XCBGenericEvent));
        XCBUnmapNotifyEvent *unev = (XCBUnmapNotifyEvent *)&ev;
        /* let event handler handle this */
        unev->from_configure = 0;
        unev->response_type = XCB_UNMAP_NOTIFY;
        unev->event = _wm.root;
        unev->window = win;
        XCBSendEvent(_wm.dpy, _wm.root, 0, XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char *)&ev);
    }
}

void
managerequest(XCBWindow win, XCBCookie requests[ManageCookieLAST])
{
    const u32 REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    const u8 STRUT_P_LENGTH = 12;
    const u8 STRUT_LENGTH = 4;
    const u8 NO_BYTE_OFFSET = 0;
    const u8 MOTIF_WM_HINT_LENGTH = 5;

    /* Window Attributes */
    requests[ManageCookieAttributes] = 
        XCBGetWindowAttributesCookie(_wm.dpy, win);
    /* Window Geometry */
    requests[ManageCookieGeometry] = 
        XCBGetWindowGeometryCookie(_wm.dpy, win);
    /* Window Transient */
    requests[ManageCookieTransient] = 
        XCBGetTransientForHintCookie(_wm.dpy, win);
    /* Window Type(s) */
    requests[ManageCookieWType] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
    /* Window State(s) */
    requests[ManageCookieWState] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
    /* Window Size Hints */
    requests[ManageCookieSizeHint] = 
        XCBGetWMNormalHintsCookie(_wm.dpy, win);
    /* Window WM Hints */
    requests[ManageCookieWMHints] = 
        XCBGetWMHintsCookie(_wm.dpy, win);
    /* Window Class/Instance */
    requests[ManageCookieClass] = 
        XCBGetWMClassCookie(_wm.dpy, win);
    /* Window WM Protocol(s) */
    requests[ManageCookieWMProtocol] = 
        XCBGetWMProtocolsCookie(_wm.dpy, win, wmatom[WMProtocols]);
    /* Window Strut */
    requests[ManageCookieStrut] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
    /* Window StrutP */
    requests[ManageCookieStrutP] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
    /* Window NetWMName */
    requests[ManageCookieNetWMName] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
    /* Window WMName */
    requests[ManageCookieWMName] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
    /* Window Pid */
    requests[ManageCookiePid] = 
        XCBGetPidCookie(_wm.dpy, win, netatom[NetWMPid]);
    /* Window Icon */
    requests[ManageCookieIcon] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_CARDINAL);
    /* Window Motif */
    requests[ManageCookieMotif] = 
        XCBGetWindowPropertyCookie(_wm.dpy, win, motifatom, NO_BYTE_OFFSET, MOTIF_WM_HINT_LENGTH, False, motifatom);
}

Client *
managereply(XCBWindow win, XCBCookie requests[ManageCookieLAST])
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
    
    const u16 bw = 0;
    const u32 bcol = 0;
    const u8 showdecor = 1;

    Monitor *m = NULL;
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
    XCBWindowProperty *motifreply = NULL;

    /* we do it here before, because we are waiting for replies and for more memory. */
    c = createclient();

    /* wait for replies */
    waattributes = XCBGetWindowAttributesReply(_wm.dpy, requests[ManageCookieAttributes]);
    wg = XCBGetWindowGeometryReply(_wm.dpy, requests[ManageCookieGeometry]);
    transstatus = XCBGetTransientForHintReply(_wm.dpy, requests[ManageCookieTransient], &trans); trans *= !!transstatus;
    wtypeunused = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieWType]);
    stateunused = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieWState]);
    hintstatus = XCBGetWMNormalHintsReply(_wm.dpy, requests[ManageCookieSizeHint], &hints);
    wmh = XCBGetWMHintsReply(_wm.dpy, requests[ManageCookieWMHints]);
    clsstatus = XCBGetWMClassReply(_wm.dpy, requests[ManageCookieClass], &cls);
    wmprotocolsstatus = XCBGetWMProtocolsReply(_wm.dpy, requests[ManageCookieWMProtocol], &wmprotocols);
    strutreply = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieStrut]);
    strutpreply = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieStrutP]);
    netwmnamereply = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieNetWMName]);
    wmnamereply = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieWMName]);
    iconreply = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieIcon]);
    pid = XCBGetPidReply(_wm.dpy, requests[ManageCookiePid]);
    motifreply = XCBGetWindowPropertyReply(_wm.dpy, requests[ManageCookieMotif]);

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

    /* this sets up the desktop which is quite important for some operations */
    clientinittrans(c, trans);

    clientinitgeom(c, wg);
    clientinitwtype(c, wtypeunused);
    clientinitwstate(c, stateunused);
    updatewindowprotocol(c, wmprotocolsstatus ? &wmprotocols : NULL);
    getnamefromreply(netwmnamereply, &netwmname);
    getnamefromreply(wmnamereply, &wmname);
    setfloating(c, !!trans);
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
    updatemotifhints(c, motifreply);
    updateicon(c, iconreply);
    XCBSelectInput(_wm.dpy, win, inputmask);
    grabbuttons(c, 0);

    m = c->desktop->mon;

    attach(c);
    attachstack(c);
    attachfocus(c);

    updateclientlist(win, ClientListAdd);
    setclientstate(c, XCB_WINDOW_NORMAL_STATE);

    HASH_ADD_INT(m->__hash, win, c);

    if(DOCKEDINITIAL(c))
    {
        if(ISFLOATING(c))
        {   setfloating(c, 0);
        }
    }
    else
    {
        /* some windows (like st) dont mean to be "floating" but rather are a side effect of their own calculation(s),
         * So we check if its in the corner, and assume its not meant to be floating.
         */
        if((c->x == m->wx && c->y == m->wy) || (c->x == m->mx && c->y == m->my))
        {   
            if(ISFLOATING(c))   
            {   setfloating(c, 0);
            }
        }
        /* else its some sort of popup and just leave floating */
        else
        {
            if(!ISFLOATING(c))
            {   setfloating(c, 1);
            }
        }
    }
    /* inherit previous client state */
    if(c->desktop && c->desktop->sel)
    {   setfullscreen(c, ISFULLSCREEN(c->desktop->sel) || ISFULLSCREEN(c));
    }

    /* propagates border_width, if size doesn't change */
    configure(c);

    /* if its a new bar we dont want to return it as the monitor now manages it */
    if(!checknewbar(m, c, strut || strutp))
    {   c = NULL;
    }
    goto CLEANUP;
FAILURE:
    free(c);
    c = NULL;
    goto CLEANUP;
CLEANUP:
    /* reply cleanup */
    free(waattributes);
    free(wg);
    free(wtypeunused);
    free(stateunused);
    free(wmh);
    XCBWipeGetWMClass(&cls);
    XCBWipeGetWMProtocols(&wmprotocols);
    free(strutreply);
    free(strutpreply);
    free(netwmnamereply);
    free(wmnamereply);
    free(iconreply);
    free(motifreply);
    return c;
}

void
maximize(Client *c)
{
    maximizehorz(c);
    maximizevert(c);
    DEBUG("Maximized: %u", c->win);
}

void
maximizehorz(Client *c)
{
    const Monitor *m = c->desktop->mon;
    const i16 x = m->wx;
    const i16 y = c->y;
    const u16 w = m->ww - (WIDTH(c) - c->w);
    const u16 h = c->h;
    resize(c, x, y, w, h, 0);
}

void
maximizevert(Client *c)
{
    const Monitor *m = c->desktop->mon;
    const i16 x = c->x;
    const i16 y = m->my;
    const u16 w = c->w;
    const u16 h = m->wh - (HEIGHT(c) - c->h);
    resize(c, x, y, w, h, 0);
}

Client *
nextclient(Client *c)
{
    return c ? c->next : c;
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
nexttiled(Client *c)
{   
    for(; c && (ISFLOATING(c) || !ISVISIBLE(c)); c = nextstack(c));
    return c;
}

Client *
nextvisible(Client *c)
{
    for(; c && !ISVISIBLE(c); c = c->next);
    return c;
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
resize(Client *c, i32 x, i32 y, i32 width, i32 height, uint8_t interact)
{
    if(applysizehints(c, &x, &y, &width, &height, interact))
    {   resizeclient(c, x, y, width, height);
    }
}

void 
resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    u32 mask = 0;

    if(c->x != x)
    {   
        c->oldx = c->x;
        c->x = x;
        mask |= XCB_CONFIG_WINDOW_X;
    }
    if(c->y != y)
    {
        c->oldy = c->y;
        c->y = y;
        mask |= XCB_CONFIG_WINDOW_Y;
    }
    if(c->w != width)
    {
        c->oldw = c->w;
        c->w = width;
        mask |= XCB_CONFIG_WINDOW_WIDTH;
    }
    if(c->h != height)
    {   
        c->oldh = c->h;
        c->h = height;
        mask |= XCB_CONFIG_WINDOW_HEIGHT;
    }

    XCBWindowChanges changes =
    {   
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };

    /* Process resize requests only to visible clients as to. 1.) Save resources, no need to handle non visible windows.
     * 2.) Incase that the window does get visible make it not appear to be movable (different desktop).
     * 3.) Prevent the window from moving itself back into view, when it should be hidden.
     * 4.) Incase a window does want focus, we switch to that desktop respectively and let showhide() do the work.
     */
    if(ISVISIBLE(c))
    {
        if(mask)
        {   XCBConfigureWindow(_wm.dpy, c->win, mask, &changes);
        }
    }
    configure(c);
}

void
sendprotocolevent(Client *c, XCBAtom proto)
{
    XCBGenericEvent ev;
    memset(&ev, 0, sizeof(XCBGenericEvent));
    XCBClientMessageEvent *cev = (XCBClientMessageEvent *)&ev;
    cev->type = wmatom[WMProtocols];
    cev->response_type = XCB_CLIENT_MESSAGE;
    cev->window = c->win;
    cev->format = 32;
    cev->data.data32[0] = proto;
    cev->data.data32[1] = XCB_CURRENT_TIME;
    XCBSendEvent(_wm.dpy, c->win, False, XCB_NONE, (const char *)&ev);
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
    if(!DISABLEBORDER(c))
    {
        c->oldbw = c->bw;
        c->bw = border_width;
        XCBSetWindowBorderWidth(_wm.dpy, c->win, c->bw);
        configure(c);
    }
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
    /* Due to windows only having 1 map state we can set this without needing to replace other data */
    const i32 data[2] = { state, XCB_NONE };
    XCBChangeProperty(_wm.dpy, c->win, wmatom[WMState], wmatom[WMState], 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}

void
setclientwtype(Client *c, XCBAtom atom, u8 state)
{
    /* TODO manage race conditions without needing to lock the server */
    const u8 _delete = !state;
    const XCBWindow win = c->win;
    const u32 NO_BYTE_OFFSET = 0L;
    const u32 REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;

    XCBCookie cookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie);
    void *data = NULL;
    u32 len = 0;
    u32 propmode = XCB_PROP_MODE_REPLACE;
    if(prop)
    {
        XCBAtom *atoms = XCBGetPropertyValue(prop);
        const uint32_t ATOM_LENGTH = XCBGetPropertyValueLength(prop, sizeof(XCBAtom));

        u32 i;
        u32 offset = 0;
        u8 set = 0;
        for(i = 0; i < ATOM_LENGTH; ++i)
        {
            if(atoms[i] == atom)
            {   
                offset = i;
                set = 1;
                break;
            }
        }

        if(set)
        {
            if(_delete)
            {
                for(i = offset; i < ATOM_LENGTH - 1; ++i)
                {   atoms[i] = atoms[i + 1];
                }
                data = atoms;
                len = ATOM_LENGTH - 1;
            }
            else  /* atom already exists do nothing */
            {   
                free(prop);
                return;
            }
        }
        else
        {
            if(_delete)     /* prop not found mark as already deleted */
            {   
                free(prop);
                return;
            }
            else    /* set propmode to append cause we didnt find it */
            {   
                propmode = XCB_PROP_MODE_APPEND;
                len = 1;
                data = &atom;
            }
        }
    }
    else
    {   
        len = 1;
        data = &atom;
    }
    XCBChangeProperty(_wm.dpy, win, netatom[NetWMWindowType], XCB_ATOM_ATOM, 32, propmode, (const char *)data, len);
}

void
setclientnetstate(Client *c, XCBAtom atom, u8 state)
{
    /* TODO manage race conditions without needing to lock the server */
    const u8 _delete = !state;
    const XCBWindow win = c->win;
    const u32 NO_BYTE_OFFSET = 0L;
    const u32 REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;

    XCBCookie cookie = XCBGetWindowPropertyCookie(_wm.dpy, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie);
    void *data = NULL;
    u32 len = 0;
    u32 propmode = XCB_PROP_MODE_REPLACE;
    if(prop)
    {
        XCBAtom *atoms = XCBGetPropertyValue(prop);
        const uint32_t ATOM_LENGTH = XCBGetPropertyValueLength(prop, sizeof(XCBAtom));

        u32 i;
        u32 offset = 0;
        u8 set = 0;
        for(i = 0; i < ATOM_LENGTH; ++i)
        {
            if(atoms[i] == atom)
            {   
                offset = i;
                set = 1;
                break;
            }
        }

        if(set)
        {
            if(_delete)
            {
                /* this gets optimized to memmove, cool!
                 * GCC v14.1.1 -Ou
                 */
                for(i = offset; i < ATOM_LENGTH - 1; ++i)
                {   atoms[i] = atoms[i + 1];
                }
                data = atoms;
                len = ATOM_LENGTH - 1;
            }
            else  /* atom already exists do nothing */
            {
                free(prop);
                return;
            }
        }
        else
        {
            if(_delete)     /* prop not found mark as already deleted */
            {   
                free(prop);
                return;
            }
            else    /* set propmode to append cause we didnt find it */
            {   
                propmode = XCB_PROP_MODE_APPEND;
                len = 1;
                data = &atom;
            }
        }
    }
    else
    {   
        len = 1;
        data = &atom;
    }
    XCBChangeProperty(_wm.dpy, win, netatom[NetWMState], XCB_ATOM_ATOM, 32, propmode, (const char *)data, len);
}

void
setdisableborder(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _FSTATE_DISABLE_BORDER, !!state);
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
    if(state && !ISFULLSCREEN(c))
    {
        setclientnetstate(c, netatom[NetWMStateFullscreen], 1);
        setborderwidth(c, c->bw);
        setborderwidth(c, 0);
    }
    else if(!state && ISFULLSCREEN(c))
    {
        setclientnetstate(c, netatom[NetWMStateFullscreen], 0);
        setborderwidth(c, c->oldbw);
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
    {   
        setclientstate(c, XCB_WINDOW_ICONIC_STATE);
        setwtypemapiconic(c, 1);
    }
    else
    {   
        setclientstate(c, XCB_WINDOW_NORMAL_STATE);
        setwtypemapnormal(c, 1);
    }
    SETFLAG(c->wstateflags, _STATE_HIDDEN, !!state);
}

void 
setkeepfocus(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _FSTATE_KEEP_FOCUS, !!state);
}

void
setmaximizedvert(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_MAXIMIZED_VERT, !!state);
}

void
setmaximizedhorz(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_MAXIMIZED_HORZ, !!state);
}

void
setshaded(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_SHADED, !!state);
}

void
setmodal(Client *c, uint8_t state)
{
    SETFLAG(c->wstateflags, _STATE_MODAL, !!state);
}

void
setoverrideredirect(Client *c, uint8_t state)
{
    SETFLAG(c->flags, _FSTATE_OVERRIDE_REDIRECT, !!state);
}

void
setsticky(Client *c, u8 state)
{
    SETFLAG(c->wstateflags, _STATE_STICKY, !!state);
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
showhide(Client *c)
{
    const Monitor *m = c->desktop->mon;
    if(ISVISIBLE(c))
    {   
        XCBMoveResizeWindow(_wm.dpy, c->win, c->x, c->y, c->w, c->h);
        setclientstate(c, XCB_WINDOW_NORMAL_STATE);
        setwtypemapiconic(c, 0);
    }
    else
    {
        const i16 x = -c->w - m->mx;
        setclientstate(c, XCB_WINDOW_ICONIC_STATE);
        setwtypemapiconic(c, 1);
        XCBMoveResizeWindow(_wm.dpy, c->win, x, c->y, c->w, c->h);
    }
}

void
unfocus(Client *c, uint8_t setfocus)
{
    if(!c)   
    {   return;
    }
    grabbuttons(c, 0);
    XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);
    if(setfocus)
    {   
        XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
    }
    SETFLAG(c->wstateflags, _STATE_FOCUSED, 0);
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
    if(!c)
    {   return;
    }
    Desktop *desk = c->desktop;
    const XCBWindow win = c->win;

    if(desk->mon->bar == c)
    {   desk->mon->bar = NULL;
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
    updateclientlist(win, ClientListRemove);
    /* no need to arrange fully cause client is not mapped anymore */
    arrange(desk);
    cleanupclient(c);
    DEBUG("Unmanaged: [%u]", win);
}

void 
unmaximize(Client *c)
{
    unmaximizevert(c);
    unmaximizehorz(c);
    DEBUG("Umaximized: [%u]", c->win);
}

void 
unmaximizehorz(Client *c)
{
    const i16 x = c->oldx;
    const i16 y = c->y;
    const u16 w = c->oldw;
    const u16 h = c->h;

    if(DOCKEDHORZ(c))
    {   
        /* if never maximized */
        if(WASDOCKEDHORZ(c))
        {   resize(c, x / 2, y, w / 2, h, 0);
        }
        else
        {   resize(c, x, y, w, h, 0);
        }
    }
    else
    {   DEBUG("Client already unmaxed horz: [%u]", c->win);
    }
}

void 
unmaximizevert(Client *c)
{
    const i16 x = c->x;
    const i16 y = c->oldy;
    const u16 w = c->w;
    const u16 h = c->oldh;
    if(DOCKEDVERT(c))
    {
        if(WASDOCKEDVERT(c))
        {   resize(c, x, y / 2, w, h / 2, 0);
        }
        else
        {   resize(c, x, y, w, h, 0);
        }
    }
    else
    {   DEBUG("Client already unmaxed vert: [%u]", c->win);
    }
}

void
updateclass(Client *c, XCBWMClass *_class)
{
    const u32 MAX_LEN = 1024;
    if(_class)
    {
        if(_class->class_name)
        {
            const u32 CLASS_NAME_LEN = strnlen(_class->class_name, MAX_LEN) + 1;
            const size_t CLASS_NAME_SIZE = sizeof(char) * CLASS_NAME_LEN;
            char *clsname = malloc(CLASS_NAME_SIZE);
            if(clsname)
            {    
                memcpy(clsname, _class->class_name, CLASS_NAME_SIZE - sizeof(char));
                clsname[CLASS_NAME_LEN - 1] = '\0';
                free(c->classname);
                c->classname = clsname;
            }
        }
        if(_class->instance_name)
        {   
            const u32 INSTANCE_NAME_LEN = strnlen(_class->instance_name, MAX_LEN) + 1;
            const size_t INSTANCE_NAME_SIZE = sizeof(char) * INSTANCE_NAME_LEN;
            char *iname = malloc(INSTANCE_NAME_SIZE);
            if(iname)
            {
                memcpy(iname, _class->instance_name, INSTANCE_NAME_SIZE - sizeof(char));
                iname[INSTANCE_NAME_LEN - 1] = '\0';
                free(c->instancename);
                c->instancename = iname;
            }
        }
    }
}

static void 
__update_motif_decor(Client *c, uint32_t hints)
{
    /* bit definitions for MwmHints.decorations */
    const u32 DECOR_ALL = 1 << 0;
    const u32 DECOR_BORDER = 1 << 1;
    const u32 DECOR_RESIZEH = 1 << 2;
    const u32 DECOR_TITLE = 1 << 3;
    const u32 DECOR_MENU = 1 << 4;
    const u32 DECOR_MINIMIZE = 1 << 5;
    const u32 DECOR_MAXIMIZE = 1 << 6;
    if(hints & DECOR_ALL)
    {   hints |= (uint32_t)~0;
    }

    if(hints & DECOR_BORDER)
    {   setborderwidth(c, c->oldbw);
    }
    else
    {   
        setdisableborder(c, 0);
        setborderwidth(c, 0);
        setdisableborder(c, 1);
    }
    if(hints & DECOR_RESIZEH)
    {   
        /* NOP */
        ASSUME(0);
    }
    if(hints & DECOR_TITLE)
    {   setshowdecor(c, 1);
    }
    else
    {   setshowdecor(c, 0);
    }
    if(hints & DECOR_MENU)
    {  
        /* NOP */
        ASSUME(0);
    }
    if(hints & DECOR_MINIMIZE || hints & DECOR_MAXIMIZE)
    {   
        /* NOP */
        ASSUME(0);
    }
}

static void __update_motif_func(Client *c, int32_t hints)
{
    /* bit definitions for MwmHints.functions */
    const u32 FUNCS_ALL = 1 << 0;
    const u32 FUNCS_RESIZE = 1 << 1;
    const u32 FUNCS_MOVE = 1 << 2;
    const u32 FUNCS_MINIMIZE = 1 << 3;
    const u32 FUNCS_MAXIMIZE = 1 << 4;
    const u32 FUNCS_CLOSE = 1 << 5;

    if(hints & FUNCS_ALL)
    {   hints |= (int32_t)~0;
    }

    if(hints & FUNCS_RESIZE)
    {   /* NOP */
    }
    else
    {   /* IDK set fixed or something */       
    }
    if(hints & FUNCS_MOVE)
    {   /* IGNORE */   
    }
    if(hints & FUNCS_MINIMIZE)
    {   /* IGNORE */
    }
    if(hints & FUNCS_MAXIMIZE)
    {   /* IGNORE */
    }
    if(hints & FUNCS_CLOSE)
    {   /* IGNORE */
    }
}

static void __update_motif_input(Client *c, int32_t hints)
{
    /* values for MwmHints.input_mode */
    enum ___input 
    {
        INPUT_MODELESS = 0,
        INPUT_PRIMARY_MODAL = 1,
        INPUT_SYSTEM_MODAL = 2,
        INPUT_FULL_MODAL = 3,
    };

    switch(hints)
    {   
        case INPUT_PRIMARY_MODAL:
        case INPUT_SYSTEM_MODAL:
        case INPUT_FULL_MODAL:
            /* FALLTHROUGH */
            /* TODO: Add a hash to client "class" name attribute and just make it so 1 primary window is allowed 
             * AKA just HASH the primary class name to be urgent/active window 
             */
            seturgent(c, 1);
            break;

        case INPUT_MODELESS:
            /* FALLTHROUGH */
        default: 
            break;
    }
}

static void __update_motif_status(Client *c, int32_t hints)
{
    /* bit definitions for MwmHints.status */
    const u32 STATUS_TEAROFF_WIDOW = 1 << 0;
    if(hints & STATUS_TEAROFF_WIDOW)
    {   setmodal(c, 1);
    }
}


void
updatemotifhints(Client *c, XCBWindowProperty *motifprop)
{
    /* bit definitions for MwmHints.flags */
    const u32 HINTS_FUNCTIONS = 1 << 0;
    const u32 HINTS_DECORATION = 1 << 1;
    const u32 HINTS_INPUT_MODE = 1 << 2;
    const u32 HINTS_STATUS = 1 << 3;

    if(motifprop)
    {
        MotifWmHints *hints = XCBGetPropertyValue(motifprop);
        uint32_t len = XCBGetPropertyValueLength(motifprop, sizeof(MotifWmHints));
        if(hints && len == 1)
        {   
            if(hints->flags & HINTS_DECORATION)
            {   __update_motif_decor(c, hints->decorations);
            }
            if(hints->flags & HINTS_FUNCTIONS)
            {   __update_motif_func(c, hints->functions);
            }
            if(hints->flags & HINTS_INPUT_MODE)
            {   __update_motif_input(c, hints->input_mode);
            }
            if(hints->flags & HINTS_STATUS)
            {   __update_motif_status(c, hints->status);
            }
        }
    }
}

void
updatesizehints(Client *c, XCBSizeHints *size)
{
    const int UNINITIALIZED = 0;
    i32 basew = UNINITIALIZED;
    i32 baseh = UNINITIALIZED;
    i32 minw = UNINITIALIZED;
    i32 minh = UNINITIALIZED;
    i32 maxw = UNINITIALIZED;
    i32 maxh = UNINITIALIZED;
    i32 incw = UNINITIALIZED;
    i32 inch = UNINITIALIZED;       
    float mina = (float)UNINITIALIZED + 0.0f;   /* make sure sign is positive */
    float maxa = (float)UNINITIALIZED + 0.0f;   /* make sure sign is positive */

    /* size is uninitialized, ensure that size.flags aren't used */
    if(!size->flags)
    {   size->flags = XCB_SIZE_HINT_P_SIZE;
    }
    if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        minw = size->min_width;
        minh = size->min_height;
    }
    else if(size->flags & XCB_SIZE_HINT_P_BASE_SIZE)
    {   
        minw = size->base_width;
        minh = size->base_height;
    }

    if(size->flags & XCB_SIZE_HINT_P_BASE_SIZE)
    {
        basew = size->base_width;
        baseh = size->base_height;
    }
    else if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {   
        minw = size->min_width;
        minh = size->min_height;
    }

    if(size->flags & XCB_SIZE_HINT_P_RESIZE_INC)
    {
        incw = size->width_inc;
        inch = size->height_inc;
    }
    if(size->flags & XCB_SIZE_HINT_P_MAX_SIZE)
    {
        maxw = size->max_width;
        maxh = size->max_height;
    }

    if(size->flags & XCB_SIZE_HINT_P_ASPECT)
    {
        mina = (float)size->min_aspect_den / (size->min_aspect_num + !size->min_aspect_den);
        maxa = (float)size->max_aspect_num / (size->max_aspect_den + !size->max_aspect_num);
        mina = fabsf(mina);
        maxa = fabsf(maxa);
    }
    /* clamp */
    minw = MIN(minw, UINT16_MAX);
    minh = MIN(minh, UINT16_MAX);
    maxw = MIN(maxw, UINT16_MAX);
    maxh = MIN(maxh, UINT16_MAX);
    basew = MIN(basew, UINT16_MAX);
    baseh = MIN(baseh, UINT16_MAX);
    (void)mina;
    (void)maxa;
    inch = MIN(inch, UINT16_MAX);
    incw = MIN(incw, UINT16_MAX);

    c->minw = minw;
    c->minh = minh;
    c->maxw = maxw;
    c->maxh = maxh;
    c->basew = basew;
    c->baseh = baseh;
    c->mina = mina;
    c->maxa = maxa;
    c->inch = inch;
    c->incw = incw;
}

void
updatetitle(Client *c, char *netwmname, char *wmname)
{
    if(c->wmname != wmname)
    {   free(c->wmname);
        c->wmname = NULL;
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
        if(toggle)
        {   
            if(DOCKEDHORZ(c))
            {   
                unmaximizehorz(c);
                setmaximizedhorz(c, 0);
            }
            else
            {   
                maximizehorz(c);
                setmaximizedhorz(c, 1);
            }
        }
        else
        {   
            if(add_remove_toggle)
            {   maximizehorz(c);
            }
            else
            {   unmaximizehorz(c);
            }
            setmaximizedhorz(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateMaximizedVert])
    {
        if(toggle)
        {
            if(DOCKEDVERT(c))
            {   
                unmaximizevert(c);
                setmaximizedvert(c, 0);
            }
            else
            {   
                maximizevert(c);
                setmaximizedvert(c, 1);
            }
        }
        else
        {
            if(add_remove_toggle)
            {   maximizevert(c);
            }
            else
            {   unmaximizevert(c);
            }
            setmaximizedvert(c, add_remove_toggle);
        }
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
