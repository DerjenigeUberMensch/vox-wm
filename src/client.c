#include <math.h> /* fabsf() */

#include "main.h"
#include "keybinds.h"
#include "hashing.h"

#include <string.h>

extern WM _wm;
extern XCBAtom netatom[];
extern XCBAtom wmatom[];
extern XCBAtom gtkatom[];
extern XCBAtom motifatom;


/* Macro definitions */

u16 OLDWIDTH(Client *c)         { return (c->oldw + (c->bw * 2)); }
u16 OLDHEIGHT(Client *c)        { return (c->oldw + (c->bw * 2)); }
u16 WIDTH(Client *c)            { return (c->w + (c->bw * 2)); }
u16 HEIGHT(Client *c)           { return (c->h + (c->bw * 2)); } 
/* Our custom states */
u32 ISALWAYSONTOP(Client *c)    { return c->ewmhflags & WStateFlagAbove; }
u32 ISALWAYSONBOTTOM(Client *c) { return c->ewmhflags & WStateFlagBelow; }
u32 WASFLOATING(Client *c)      { return c->flags & ClientFlagWasFloating; }
u32 ISFLOATING(Client *c)       { return c->flags & ClientFlagFloating; }
u32 ISOVERRIDEREDIRECT(Client *c) { return c->flags & ClientFlagOverrideRedirect; }
u32 KEEPFOCUS(Client *c)        { return c->flags & ClientFlagKeepFocus; }
u32 DISABLEBORDER(Client *c)    { return c->flags & ClientFlagDisableBorder; }

u32 WASDOCKEDVERT(Client *c)    {   const i16 wy = c->desktop->mon->wy;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 y = c->oldy;
                                    const u16 h = OLDHEIGHT(c);
                                    return (wy == y) && (wh == h);
                                }
u32 WASDOCKEDHORZ(Client *c)    {   const i16 wx = c->desktop->mon->wx;
                                    const u16 ww = c->desktop->mon->ww;
                                    const i16 x = c->oldx;
                                    const u16 w = OLDWIDTH(c);
                                    return (wx == x) && (ww == w);
                                }

u32 WASDOCKED(Client *c)        { return WASDOCKEDVERT(c) & WASDOCKEDHORZ(c); }

u32 DOCKEDVERT(Client *c)       {   const i16 wy = c->desktop->mon->wy;
                                    const u16 wh = c->desktop->mon->wh;
                                    const i16 y = c->y;
                                    const u16 h = HEIGHT(c);
                                    return (wy == y) && (wh == h);
                                }

u32 DOCKEDHORZ(Client *c)       {   const i16 wx = c->desktop->mon->wx;
                                    const u16 ww = c->desktop->mon->ww;
                                    const i16 x = c->x;
                                    const u16 w = WIDTH(c);
                                    return (wx == x) && (ww == w);
                                }
u32 DOCKED(Client *c)           { return DOCKEDVERT(c) & DOCKEDHORZ(c); }
u32 COULDBEFLOATINGGEOM(Client *c)  
                                {
                                    const Monitor *m = c->desktop->mon;
                                    /* If the window didnt have any states, steam... 
                                    * Check if its resonably small enough to be a floating window 
                                    */
                                    const float FLOAT_SIZE_MIN = .65f;
                                    u8 floatw;
                                    u8 floath;
                                    floatw = (c->w <= m->ww * FLOAT_SIZE_MIN || c->w <= m->mw * FLOAT_SIZE_MIN);
                                    floath =  (c->h <= m->wh * FLOAT_SIZE_MIN || c->h <= m->mh * FLOAT_SIZE_MIN);

                                    u32 ret = floatw || floath;
                                    /* If its not resonably small check a much stricter guideline,
                                     * But plausible if say they moved it off to the bottom of the screen or something 
                                     */
                                    const float FLOAT_SIZE_MIN_STRICT = .75f;
                                    const float FLOAT_OFFSET_MIN_STRICT = .65f;
                                    const float FLOAT_OFFSET_MAX_STRICT = .95f;     /* dont want it offscreen */
                                    u8 floatx;
                                    u8 floaty;
                                    floatw = (c->w <= m->ww * FLOAT_SIZE_MIN_STRICT || c->w <= m->mw * FLOAT_SIZE_MIN_STRICT);
                                    floath = (c->h <= m->wh * FLOAT_SIZE_MIN_STRICT || c->h <= m->mh * FLOAT_SIZE_MIN_STRICT);
                                    floatx = BETWEEN(c->x, (m->wx + m->ww) * FLOAT_OFFSET_MIN_STRICT, (m->wx + m->ww) * FLOAT_OFFSET_MAX_STRICT);
                                    floaty = BETWEEN(c->y, (m->wy + m->wh) * FLOAT_OFFSET_MIN_STRICT, (m->wy + m->wh) * FLOAT_OFFSET_MAX_STRICT);
                                    if(!ret)
                                    {   ret = (floatw && floatx) || (floath && floaty);
                                    }
                                    return ret;
                                }
u32 COULDBEFLOATINGHINTS(Client *c)
                                {
                                    /* This check is mostly for (some) popup windows 
                                     * Mainly those which dont matter, like steams startup display, but are nice to have's.
                                     */
                                    if(ISSPLASH(c))
                                    {   Debug0("Splash Window.");
                                    }
                                    else if(ISMODAL(c))
                                    {   Debug0("Modal Window.");
                                    }
                                    else if(ISPOPUPMENU(c))
                                    {   Debug0("Popup Menu");
                                    }
                                    else if(ISDIALOG(c))
                                    {   Debug0("Dialog Menu");
                                    }
                                    else if(ISNOTIFICATION(c))
                                    {   Debug0("Notification.");
                                    }
                                    else if(ISCOMBO(c))
                                    {   Debug0("Combo Menu,");
                                    }
                                    /* This check is mostly as to not soft lock the window to always be above others */
                                    else if(ISABOVE(c))
                                    {   Debug0("AlwaysOnTop Window detected.");
                                    }
                                    /* This checks for other non dialog types that sort of work like dialog(s) if not maximized. */
                                    else if(ISUTILITY(c))
                                    {   Debug0("Util Window detected, maybe picture-in-picture?");
                                    }
                                    else
                                    {   return 0;
                                    }
                                    return !DOCKEDINITIAL(c);
                                }
u32 SHOULDBEFLOATING(Client *c) 
                                {
                                    u32 ret = 0;
                                    if(COULDBEFLOATINGHINTS(c))
                                    {   return 1;
                                    }
                                    /* Note dont check if ISFIXED(c) as games often set that option */
                                    if(COULDBEFLOATINGGEOM(c))
                                    {   
                                        Debug0("Client is small enough to be floating, but should use hints...");
                                        ret = 1;
                                    }
                                    else
                                    {   ret = 0;
                                    }
                                    /* extra checks to make sure its floating */
                                    if(ret)
                                    {
                                        /* If they dont have a classname/instancename then likely they are single instance windows */
                                        if(!c->classname || !c->instancename)
                                        {   ret = 0;
                                        }
                                        /* Some windows do set their classname/instancename but to the same string which means one of the following.
                                         * A.) Its the main window, which we shouldnt make floating (duh).
                                         * B.) It has subwindows but again see above.
                                         * C.) It sets this to all windows and doesnt have any subwindows.
                                         * D.) (rarely) Its broken, but probably will be fixed later if their developer cares enough.
                                         */
                                        else if(!strcmp(c->classname, c->instancename))
                                        {   ret = 0;
                                        }
                                    }
                                    return ret;
                                }
/* This covers some apps being able to DragWindow/ResizeWindow, in toggle.c
 * (semi-frequently) a user might "accidentally" click on them (me) and basically we dont want that window to be floating because of that user error.
 * So this is a leeway sort function.
 */
u32 SHOULDMAXIMIZE(Client *c)   {
                                    if(DOCKED(c))
                                    {   return 0;
                                    }
                                    Monitor *m = c->desktop->mon;
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


                                    /* leeway, pixels */
                                    const u8 LWY = 2;
                                    const u8 iww = (BETWEEN(w, ww + LWY, ww - LWY)) && (BETWEEN(h, wh + LWY, wh - LWY));
                                    const u8 iwb = (BETWEEN(w1, ww + LWY, ww - LWY)) && (BETWEEN(h1, wh + LWY, wh - LWY));
                                    const u8 imw = (BETWEEN(h, mh + LWY, mh - LWY)) && (BETWEEN(w, mw + LWY, mw - LWY));
                                    const u8 imb = (BETWEEN(h1, mh + LWY, mh - LWY)) && (BETWEEN(h1, mw + LWY, mh + LWY));

                                    /* leeway, pixels */
                                    const u8 LWYC = 15;
                                    const u8 iwx = (BETWEEN(x, wx + LWYC, wx - LWYC)) && (BETWEEN(y, wy + LWYC, wy - LWYC));
                                    const u8 imx = (BETWEEN(x, mx + LWYC, mx - LWYC)) && (BETWEEN(y, my + LWYC, my - LWYC));

                                    return
                                        (iwx && (iww || iwb))
                                        ||
                                        (imx && (imw || imb))
                                    ;
                                }

/* used in manage */
u32 DOCKEDINITIAL(Client *c)    {   Monitor *m = c->desktop->mon;
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

u32 ISFIXED(Client *c)          { return (c->minw != 0) && (c->minh != 0) && (c->minw == c->maxw) && (c->minh == c->maxh); }
u32 ISURGENT(Client *c)         { return c->ewmhflags & WStateFlagDemandAttention; }
/* flag */
u32 NEVERFOCUS(Client *c)       { return c->ewmhflags & WStateFlagNeverFocus; }
/* client state 
 * taken from i3, 
 * polybar kinda sucks at us grabbing their buttons, so we choose to never focus "dock" (generally statusbars) type windows.
 * What does polybar do?:
 * It used to break the whole WM, but that was mitagated from another i3 feature of replay pou32ing only the main button(s).
 * Now its only unusable if grabbuttons(), twice, basically the "focus" part of grabbuttons(), which is undisireable.
 */
u32 NEVERHOLDFOCUS(Client *c)   { return NEVERFOCUS(c) || ISDOCK(c);}
u32 ISMAXHORZ(Client *c)        { return WIDTH(c) == c->desktop->mon->ww; }
u32 ISMAXVERT(Client *c)        { return HEIGHT(c) == c->desktop->mon->wh; }
u32 ISVISIBLE(Client *c)        { return (c->desktop->mon->desksel == c->desktop || ISSTICKY(c)) && !ISHIDDEN(c); }
u32 SHOWDECOR(Client *c)        { return c->flags & ClientFlagShowDecor; }
u32 ISSELECTED(Client *c)       { return c->desktop->sel == c; }
        
u32 COULDBEBAR(Client *c, uint8_t strut) 
                                {
                                    const u8 sticky = !!ISSTICKY(c);
                                    const u8 isdock = !!(ISDOCK(c));
                                    const u8 above = !!ISABOVE(c); 
                                    return (sticky && strut && (above || isdock));
                                }
/* EWMH Window types */
u32 ISDESKTOP(Client *c)        { return c->ewmhflags & WTypeFlagDesktop; }
u32 ISDOCK(Client *c)           { return c->ewmhflags & WTypeFlagDock; }
u32 ISTOOLBAR(Client *c)        { return c->ewmhflags & WTypeFlagToolbar; }
u32 ISMENU(Client *c)           { return c->ewmhflags & WTypeFlagMenu; }
u32 ISUTILITY(Client *c)        { return c->ewmhflags & WTypeFlagUtility; }
u32 ISSPLASH(Client *c)         { return c->ewmhflags & WTypeFlagSplash; }
u32 ISDIALOG(Client *c)         { return c->ewmhflags & WTypeFlagDialog; }
u32 ISDROPDOWNMENU(Client *c)   { return c->ewmhflags & WTypeFlagDropdownMenu; }
u32 ISPOPUPMENU(Client *c)      { return c->ewmhflags & WTypeFlagPopupMenu; }
u32 ISTOOLTIP(Client *c)        { return c->ewmhflags & WTypeFlagTooltip; }
u32 ISNOTIFICATION(Client *c)   { return c->ewmhflags & WTypeFlagNotification; }
u32 ISCOMBO(Client *c)          { return c->ewmhflags & WTypeFlagCombo; }
u32 ISDND(Client *c)            { return c->ewmhflags & WTypeFlagDnd; }
u32 ISNORMAL(Client *c)         { return c->ewmhflags & WTypeFlagNormal; }
u32 ISMAPICONIC(Client *c)      { return c->ewmhflags & WStateFlagMapIconic; }
u32 ISMAPNORMAL(Client *c)      { return !ISMAPICONIC(c); }
/* EWMH Window states */
u32 ISMODAL(Client *c)          { return c->ewmhflags & WStateFlagModal; }
u32 ISSTICKY(Client *c)         { return c->ewmhflags & WStateFlagSticky; }
/* DONT USE */
u32 ISMAXIMIZEDVERT(Client *c)  { return c->ewmhflags & WStateFlagMaximizedVert; }
/* DONT USE */
u32 ISMAXIMIZEDHORZ(Client *c)  { return c->ewmhflags & WStateFlagMaximizedHorz; }
u32 ISSHADED(Client *c)         { return c->ewmhflags & WStateFlagShaded; }
u32 SKIPTASKBAR(Client *c)      { return c->ewmhflags & WStateFlagSkipTaskbar; }
u32 SKIPPAGER(Client *c)        { return c->ewmhflags & WStateFlagSkipPager; }
u32 ISHIDDEN(Client *c)         { return c->ewmhflags & WStateFlagHidden; }
u32 ISFULLSCREEN(Client *c)     { return c->ewmhflags & WStateFlagFullscreen; }
u32 ISABOVE(Client *c)          { return c->ewmhflags & WStateFlagAbove; }
u32 ISBELOW(Client *c)          { return c->ewmhflags & WStateFlagBelow; }
u32 DEMANDSATTENTION(Client *c) { return c->ewmhflags & WStateFlagDemandAttention; }
u32 ISFOCUSED(Client *c)        { return c->ewmhflags & WStateFlagFocused; }
u32 WSTATENONE(Client *c)       { return c->ewmhflags == 0; }
/* WM Protocol */
u32 HASWMTAKEFOCUS(Client *c)   { return c->ewmhflags & WStateFlagWMTakeFocus; }
u32 HASWMSAVEYOURSELF(Client *c){ return c->ewmhflags & WStateFlagWMSaveYourself; }
u32 HASWMDELETEWINDOW(Client *c){ return c->ewmhflags & WStateFlagWMDeleteWindow; }

void
applygravity(const enum XCBBitGravity gravity, int32_t *x, int32_t *y, const uint32_t w, const uint32_t h, const uint32_t bw)
{
    if(!x || !y)
    {   return;
    }
    /* im a dumbass */
    switch(gravity)
    {
        case XCBNorthGravity:
            *x += w >> 1;
            *y -= bw;
            break;
        case XCBNorthWestGravity:
            *x -= bw;
            *y -= bw;
            break;
        case XCBNorthEastGravity:
            *x += w + bw;
            *y -= bw;
            break;
        case XCBWestGravity:
            *x -= bw;
            *y += h >> 1;
            break;
        case XCBEastGravity:
            *x += w + bw;
            *y += h >> 1;
            break;
        case XCBSouthWestGravity:
            *x -= bw;
            *y += h + bw;
            break;
        case XCBSouthEastGravity:
            *x += w + bw;
            *y += h + bw;
            break;
        case XCBSouthGravity:
            *x += w >> 1;
            *y += h + bw;
            break;
        case XCBCenterGravity:
            *x += w >> 1;
            *y += h >> 1;
            break;
        case XCBForgetGravity:
            /* FALLTHROUGH */
        case XCBStaticGravity:
            /* FALLTHROUGH */
        default:
            Debug("Window has no gravity. [%d]", gravity);
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
clientinitcolormap(Client *c, XCBGetWindowAttributes *wa)
{
    if(wa)
    {
        c->colormap = 0;
        updatecolormap(c, wa->colormap);
    }
}

void
clientinitdecor(Client *c)
{
    Decoration *decor = c->decor;

    decor->h = 15;
    decor->w = 10;

    const u8 depth = XCB_COPY_FROM_PARENT;
    const XCBVisual visual = XCBGetScreen(_wm.dpy)->root_visual;
    const u8  class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    const u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL;

    XCBCreateWindowValueList va =
    {
        .background_pixel = ~0,
        .border_pixel = 0,
        .override_redirect = 1,
    };

    decor->win = XCBCreateWindow(_wm.dpy, _wm.root, 0, 0, c->w, c->h, 0, depth, class, visual, mask, &va);
}

void
clientinitfloat(Client *c)
{
    if(!SHOULDBEFLOATING(c))
    {   return;
    }
    const Monitor *m = c->desktop->mon;
    const u32 bw = WIDTH(c) - c->w;
    const u32 bh = HEIGHT(c) - c->h;
    /* If the window is docked for whatever reason, center it and shrink it down */
    if(DOCKEDINITIAL(c))
    {
        i32 w = m->ww / 2 - bw;
        i32 h = m->wh / 2 - bh;
        i32 x = m->wx + (w / 2) + bw;
        i32 y = m->my + (h / 2) + bh;
        resize(c, x, y, w, h, 0);
    }
    /* if its in the corner move it to center */
    else if((c->x == m->wx && c->y == m->wy) || (c->x == m->mx && c->y == m->my))
    {
        i32 x = m->wx + ((m->ww - WIDTH(c)) / 2);
        i32 y = m->wy + ((m->wh - HEIGHT(c)) / 2);
        resize(c, x, y, c->w, c->h, 0);
    }
    if(!ISFLOATING(c))
    {   setfloating(c, 1);
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
        uint32_t ATOM_LENGTH = 0;
        XCBGetPropertyValueLength(windowtypereply, sizeof(XCBAtom), &ATOM_LENGTH);
        updatewindowtypes(c, data, ATOM_LENGTH);
    }
}

void 
clientinitwstate(Client *c, XCBWindowProperty *windowstatereply)
{
    if(windowstatereply)
    {
        XCBAtom *data = XCBGetPropertyValue(windowstatereply);
        uint32_t ATOM_LENGTH = 0;
        XCBGetPropertyValueLength(windowstatereply, sizeof(XCBAtom), &ATOM_LENGTH);
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
    Decoration *decor = X11DecorCreate();
    if(!c || !decor)
    {   
        Debug0("Could not allocate memory for client (OutOfMemory).");
        Debug("Client:      %p", (void *)c);
        Debug("Decoration:  %p", (void *)decor);
        free(c);
        free(decor);
        return NULL;
    }
    c->decor = decor;
    c->x = c->y = 0;
    c->w = c->h = 0;
    c->oldx = c->oldy = 0;
    c->oldw = c->oldh = 0;
    c->ewmhflags = 0;
    c->ewmhflags = 0;
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
    if(!c || !ISVISIBLE(c) || NEVERHOLDFOCUS(c))
    {   for(c = startfocus(desk); c && !ISVISIBLE(c) && !KEEPFOCUS(c); c = nextfocus(c));
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
    Debug("Focused: [%d]", c ? c->win : 0);
}

void
grabbuttons(Client *c, uint8_t focused)
{
    /* make sure no other client steals our grab */
    xcb_grab_server(_wm.dpy);
    u16 i, j;
    /* numlock is int */
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK};
    /* somewhat taken from i3 */
    /* Always grab these to allow for replay pointer when focusing by mouse click */
    u8 gbuttons[3] = { LMB, MMB, RMB };

    /* ungrab any previously grabbed buttons that are ours */
    for(i = 0; i < LENGTH(modifiers); ++i)
    {
        if(!NEVERHOLDFOCUS(c))
        {
            for(j = 0; j < LENGTH(gbuttons); ++j)
            {   XCBUngrabButton(_wm.dpy, gbuttons[j], modifiers[i], c->win);
            }
        }
        for(j = 0; j < LENGTH(buttons); ++j)
        {   XCBUngrabButton(_wm.dpy, buttons[j].button, modifiers[i], c->win);
        }
    }
    if (!focused)
    {
        /* grab focus buttons */
        if(!NEVERHOLDFOCUS(c))
        {
            for (i = 0; i < LENGTH(gbuttons); ++i)
            {
                for (j = 0; j < LENGTH(modifiers); ++j)
                {   XCBGrabButton(_wm.dpy, gbuttons[i], modifiers[j], c->win, False, BUTTONMASK, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_SYNC, XCB_NONE, XCB_NONE);
                }
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

Client *
lastclient(Desktop *desk)
{
    return desk->clast;
}

Client *
lastfocus(Desktop *desk)
{
    return desk->flast;
}

Client *
laststack(Desktop *desk)
{
    return desk->slast;
}

Client *
lastrstack(Desktop *desk)
{
    return desk->rlast;
}

void
managerequest(XCBWindow win, XCBCookie requests[ManageCookieLAST])
{
    requests[ManageCookieAttributes] = XCBGetWindowAttributesCookie(_wm.dpy, win);
    requests[ManageCookieGeometry] = XCBGetWindowGeometryCookie(_wm.dpy, win);
    requests[ManageCookieTransient] = GetTransientCookie(_wm.dpy, win);
    requests[ManageCookieWType] = GetWindowTypeCookie(_wm.dpy, win);
    requests[ManageCookieWState] = GetWindowStateCookie(_wm.dpy, win);
    requests[ManageCookieSizeHint] = GetSizeHintsCookie(_wm.dpy, win);
    requests[ManageCookieWMHints] = GetWMHintsCookie(_wm.dpy, win);
    requests[ManageCookieClass] = GetWMClassCookie(_wm.dpy, win);
    requests[ManageCookieWMProtocol] = GetWMProtocolCookie(_wm.dpy, win);
    requests[ManageCookieStrut] = GetStrutCookie(_wm.dpy, win);
    requests[ManageCookieStrutP] = GetStrutpCookie(_wm.dpy, win);
    requests[ManageCookieNetWMName] = GetNetWMNameCookie(_wm.dpy, win);
    requests[ManageCookieWMName] = GetWMNameCookie(_wm.dpy, win);
    requests[ManageCookiePid] = GetPidCookie(_wm.dpy, win);
    requests[ManageCookieIcon] = GetIconCookie(_wm.dpy, win);
    requests[ManageCookieMotif] = GetMotifHintsCookie(_wm.dpy, win);
}

Client *
managereply(XCBWindow win, XCBCookie requests[ManageCookieLAST])
{
    /* checks */
    if(win == _wm.root)
    {   Debug("%s", "Cannot manage() root window.");
        goto DISCARD;
    }
    else if(wintoclient(win))
    {   Debug("Window already managed????: [%u]", win);
        goto DISCARD;
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
    {   Debug("Override Redirect: [%d]", win);
        /* theoredically we could manage these but they are a hastle to deal with */
        goto FAILURE;
    }

    /* this sets up the desktop which is quite important for some operations */
    clientinitcolormap(c, waattributes);
    clientinittrans(c, trans);
    clientinitgeom(c, wg);
    clientinitwtype(c, wtypeunused);
    clientinitwstate(c, stateunused);
    updatewindowprotocol(c, wmprotocolsstatus ? &wmprotocols : NULL);
    netwmname = getnamefromreply(netwmnamereply);
    wmname = getnamefromreply(wmnamereply);
    setfloating(c, !!trans);
    /* Custom stuff */
    setclientpid(c, pid);
    setborderwidth(c, bw);
    setbordercolor32(c, bcol);
    setshowdecor(c, showdecor);
    updatetitle(c, netwmname, wmname);
    updateborder(c);
    updatesizehints(c, &hints);
    if(clsstatus)
    {   updateclass(c, &cls);
    }
    updatewmhints(c, wmh);
    updatemotifhints(c, motifreply);
    updateicon(c, iconreply);
    /* check if should be floating after, all size hints and other things are set. */
    clientinitfloat(c);
    clientinitdecor(c);
    XCBSelectInput(_wm.dpy, win, inputmask);
    grabbuttons(c, 0);

    m = c->desktop->mon;

    attach(c);
    attachstack(c);
    attachfocus(c);

    updateclientlist(win, ClientListAdd);
    setclientstate(c, XCB_WINDOW_NORMAL_STATE);

    /* add to hash */
    addclienthash(c);

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
DISCARD:
    for(pid = 0; pid < ManageCookieLAST; ++pid)
    {   XCBDiscardReply(_wm.dpy, requests[pid]);
    }
    return NULL;
}

void
maximize(Client *c)
{
    maximizehorz(c);
    maximizevert(c);
    Debug("Maximized: %u", c->win);
}

void
maximizehorz(Client *c)
{
    const Monitor *m = c->desktop->mon;
    i32 x = m->wx;
    i32 y = c->y;
    i32 w = m->ww - (WIDTH(c) - c->w);
    i32 h = c->h;
    resize(c, x, y, w, h, 0);
}

void
maximizevert(Client *c)
{
    const Monitor *m = c->desktop->mon;
    i32 x = c->x;
    i32 y = m->my;
    i32 w = c->w;
    i32 h = m->wh - (HEIGHT(c) - c->h);
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

    /* Process resize requests only to visible clients as to.
     * 1.) Save resources, no need to handle non visible windows.
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
    /* only send config if changed */
    if(mask)
    {   configure(c);
    }
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
    XCBSendEvent(_wm.dpy, c->win, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
}

void
setalwaysontop(Client *c, u8 state)
{
    SETFLAG(c->ewmhflags, WStateFlagAbove, !!state);
}

void
setalwaysonbottom(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagBelow, !!state);
}

void
setborderalpha(Client *c, uint8_t alpha)
{
    c->bcol &= ~(UINT8_MAX << 24);
    c->bcol |= alpha << 24;
}

void
setbordercolor(Client *c, uint8_t red, uint8_t green, uint8_t blue)
{
    const u32 col = blue + (green << 8) + (red << 16);
    setbordercolor32(c, col);
}

void
setbordercolor32(Client *c, uint32_t col)
{   
    const u32 mask = (UINT32_MAX ^ (UINT8_MAX << 24));
    c->bcol &= mask;
    c->bcol |= col & mask;
}

void
setborderwidth(Client *c, uint16_t border_width)
{
    if(!DISABLEBORDER(c))
    {
        c->oldbw = c->bw;
        c->bw = border_width;
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
        uint32_t ATOM_LENGTH = 0;
        XCBGetPropertyValueLength(prop, sizeof(XCBAtom), &ATOM_LENGTH);

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
            {   goto CLEANUP;   
            }
        }
        else
        {
            if(_delete)     /* prop not found mark as already deleted */
            {   goto CLEANUP;   
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
    goto CLEANUP;
CLEANUP:
    free(prop);
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
        uint32_t ATOM_LENGTH = 0;
        XCBGetPropertyValueLength(prop, sizeof(XCBAtom), &ATOM_LENGTH);

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
                 * GCC v14.1.1 -O3
                 */
                for(i = offset; i < ATOM_LENGTH - 1; ++i)
                {   atoms[i] = atoms[i + 1];
                }
                data = atoms;
                len = ATOM_LENGTH - 1;
            }
            else  /* atom already exists do nothing */
            {   goto CLEANUP;
            }
        }
        else
        {
            if(_delete)     /* prop not found mark as already deleted */
            {   goto CLEANUP;   
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
    goto CLEANUP;
CLEANUP:
    free(prop);
}

void
setdisableborder(Client *c, uint8_t state)
{
    SETFLAG(c->flags, ClientFlagDisableBorder, !!state);
}

void
setclientpid(Client *c, pid_t pid)
{
    c->pid = pid;
}

void
setwtypedesktop(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagDesktop, !!state);
}

void
setwtypedialog(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagDialog, !!state);
}

void
setwtypedock(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagDock, !!state);
}

void
setwtypetoolbar(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagToolbar, !!state);
}

void
setwtypemenu(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagMenu, !!state);
}

void
setwtypeneverfocus(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagNeverFocus, !!state);
}

void
setwtypeutility(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagUtility, !!state);
}

void
setwtypesplash(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagSplash, !!state);
}

void
setwtypedropdownmenu(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagDropdownMenu, !!state);
}

void
setwtypepopupmenu(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagPopupMenu, !!state);
}

void
setwtypetooltip(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagTooltip, !!state);
}

void
setwtypenotification(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagNotification, !!state);
}

void
setwtypecombo(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagCombo, !!state);
}

void
setwtypednd(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagDnd, !!state);
}

void
setwtypenormal(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WTypeFlagNormal, !!state);
}

void
setwtypemapiconic(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagMapIconic, !!state);
}

void
setwtypemapnormal(Client *c, uint8_t state)
{
    setwtypemapiconic(c, !state);
}

void 
setwmtakefocus(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagWMTakeFocus, !!state);
}

void 
setwmsaveyourself(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagWMSaveYourself, !!state);
}

void 
setwmdeletewindow(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagWMDeleteWindow, !!state);
}

void
setskippager(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagSkipPager, !!state);
}

void
setskiptaskbar(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagSkipTaskbar, !!state);
}

void
setshowdecor(Client *c, uint8_t state)
{
    /* TODO, Implement this.
     * As of now this is not implemented but for the sake of being NetWM Compliant, 
     * We just set the decorations to be 0, (AKA No decorations)
     */
    state = 0;
    enum __FrameExtents
    {
        __FrameExtentsLW,   /* Left "decoration" Width */
        __FrameExtentsRW,   /* Right "decoration" Width */
        __FrameExtentsTW,   /* Top "decoration" Width, AKA the Height */
        __FrameExtentsBW,   /* Bottom "decoration" Width, AKA the Height */
    };
    u32 data[4] = { 0, 0, 0, 0 };
    if(state)
    {   
        if(c->decor->win)
        {   
            Decoration *decor = c->decor;
            XCBMapWindow(_wm.dpy, decor->win);   
            /* as of now not supported but eventually */
            data[0] = 0;
            data[1] = 0;
            data[2] = decor->h;
            data[3] = 0;
        }
    }
    else
    {
        if(c->decor->win)
        {   XCBUnmapWindow(_wm.dpy, c->decor->win);
        }
    }
    SETFLAG(c->flags, ClientFlagShowDecor, !!state);
    XCBChangeProperty(_wm.dpy, c->win, netatom[NetWMFrameExtents], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 4);
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
    updateborderwidth(c);
    SETFLAG(c->ewmhflags, WStateFlagFullscreen, !!state);
}

void
setfloating(Client *c, uint8_t state)
{
    SETFLAG(c->flags, ClientFlagWasFloating, !!(c->flags & ClientFlagFloating));
    SETFLAG(c->flags, ClientFlagFloating, !!state);
}

void
setfocus(Client *c)
{
    if(!NEVERHOLDFOCUS(c))
    {
        XCBSetInputFocus(_wm.dpy, c->win, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBChangeProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&(c->win), 1);
        setclientnetstate(c, netatom[NetWMStateFocused], 1);
        SETFLAG(c->ewmhflags, WStateFlagFocused, 1);
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
    SETFLAG(c->ewmhflags, WStateFlagHidden, !!state);
}

void 
setkeepfocus(Client *c, uint8_t state)
{
    SETFLAG(c->flags, ClientFlagKeepFocus, !!state);
}

void
setmaximizedvert(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagMaximizedVert, !!state);
}

void
setmaximizedhorz(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagMaximizedHorz, !!state);
}

void
setshaded(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagShaded, !!state);
}

void
setmodal(Client *c, uint8_t state)
{
    SETFLAG(c->ewmhflags, WStateFlagModal, !!state);
}

void
setoverrideredirect(Client *c, uint8_t state)
{
    SETFLAG(c->flags, ClientFlagOverrideRedirect, !!state);
}

void
setsticky(Client *c, u8 state)
{
    SETFLAG(c->ewmhflags, WStateFlagSticky, !!state);
}

void
seturgent(Client *c, uint8_t state) 
{
    XCBCookie wmhcookie = XCBGetWMHintsCookie(_wm.dpy, c->win);
    XCBWMHints *wmh = XCBGetWMHintsReply(_wm.dpy, wmhcookie);
    SETFLAG(c->ewmhflags, WStateFlagDemandAttention, !!state);
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
    i16 x;
    if(ISVISIBLE(c))
    {   
        x = c->x;
        setclientstate(c, XCB_WINDOW_NORMAL_STATE);
        setwtypemapiconic(c, 0);
    }
    else
    {
        x = -c->w - m->mx;
        setclientstate(c, XCB_WINDOW_ICONIC_STATE);
        setwtypemapiconic(c, 1);
    }
    XCBMoveResizeWindow(_wm.dpy, c->win, x, c->y, c->w, c->h);
}

Client *
startclient(Desktop *desk)
{
    return desk->clients;
}

Client *
startfocus(Desktop *desk)
{
    return desk->focus;
}

Client *
startstack(Desktop *desk)
{
    return desk->stack;
}

Client *
startrstack(Desktop *desk)
{
    return desk->rstack;
}

void
unfocus(Client *c, uint8_t setfocus)
{
    if(!c)   
    {   return;
    }
    grabbuttons(c, 0);
    XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);
    setclientnetstate(c, netatom[NetWMStateFocused], 0);
    if(setfocus)
    {   
        XCBSetInputFocus(_wm.dpy, _wm.root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm.dpy, _wm.root, netatom[NetActiveWindow]);
    }
    SETFLAG(c->ewmhflags, WStateFlagFocused, 0);
}

void
updateicon(Client *c, XCBWindowProperty *iconprop)
{
    free(c->icon);
    c->icon = geticonprop(iconprop);
}

void
updateborder(Client *c)
{
    updateborderwidth(c);
    updatebordercol(c);
}

void
updatebordercol(Client *c)
{
    XCBSetWindowBorder(_wm.dpy, c->win, c->bcol);
}

void
updateborderwidth(Client *c)
{
    XCBSetWindowBorderWidth(_wm.dpy, c->win, c->bw);
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
    /* prevent dangling pointer here woops */
    if(desk->sel == c)
    {   desk->sel = NULL;
    }
    if(!destroyed)
    {   
        /* TODO causes alot of errors for some reason even if its not "destroyed" */
    }
    delclienthash(c);
    detachcompletely(c);
    /* Destroy colormap */
    updatecolormap(c, 0);
    updateclientlist(win, ClientListRemove);
    cleanupclient(c);
    Debug("Unmanaged: [%u]", win);
}

void 
unmaximize(Client *c)
{
    unmaximizevert(c);
    unmaximizehorz(c);
    Debug("Umaximized: [%u]", c->win);
}

void 
unmaximizehorz(Client *c)
{
    i32 x = c->oldx;
    i32 y = c->y;
    i32 w = c->oldw;
    i32 h = c->h;

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
    {   Debug("Client already unmaxed horz: [%u]", c->win);
    }
}

void 
unmaximizevert(Client *c)
{
    i32 x = c->x;
    i32 y = c->oldy;
    i32 w = c->w;
    i32 h = c->oldh;
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
    {   Debug("Client already unmaxed vert: [%u]", c->win);
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

void
updatecolormap(Client *c, XCBColormap colormap)
{
    if(c)
    {
        if(c->colormap)
        {   XCBUninstallColormap(_wm.dpy, c->colormap);
        }
        if(colormap)
        {   XCBInstallColormap(_wm.dpy, colormap);
        }
        c->colormap = colormap;
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
    {   
        setborderwidth(c, c->oldbw);
        updateborderwidth(c);
    }
    else
    {   
        setdisableborder(c, 0);
        setborderwidth(c, 0);
        updateborderwidth(c);
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
        uint32_t len = 0;
        XCBGetPropertyValueLength(motifprop, sizeof(MotifWmHints), &len);
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
    i32 gravity = UNINITIALIZED;

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

    if(size->flags & XCB_SIZE_HINT_P_WIN_GRAVITY)
    {   gravity = size->win_gravity;
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
    gravity = MIN(gravity, XCBStaticGravity);

    /* cleanse impossible sizes */
    minw = MAX(minw, 0);
    minh = MAX(minh, 0);
    maxw = MAX(maxw, 0);
    maxh = MAX(maxh, 0);
    basew = MAX(basew, 0);
    baseh = MAX(baseh, 0);
    mina = MAX(mina, 0);
    maxa = MAX(maxa, 0);
    inch = MAX(inch, 0);
    incw = MAX(incw, 0);
    gravity = MAX(gravity, 0);

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
    c->gravity = gravity;
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
        {   setalwaysontop(c, !ISALWAYSONTOP(c));
        }
        else
        {   setalwaysontop(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateDemandAttention])
    {
        if(toggle)
        {   seturgent(c, !ISURGENT(c));
        }
        else
        {   seturgent(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateFullscreen])
    {
        if(toggle)
        {   setfullscreen(c, !ISFULLSCREEN(c));
        }
        else
        {   setfullscreen(c, add_remove_toggle);
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
        {   setsticky(c, !ISSTICKY(c));
        }
        else
        {   setsticky(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateBelow])
    {   
        /* this is a wierd state to even configure so idk */
        if(toggle)
        {       setalwaysonbottom(c, !ISALWAYSONBOTTOM(c));
        }
        else
        {   setalwaysonbottom(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateSkipTaskbar])
    {   
        if(toggle)
        {   setskiptaskbar(c, !SKIPTASKBAR(c));
        }
        else
        {   setskiptaskbar(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateSkipPager])
    {
        if(toggle)
        {   setskippager(c, !SKIPPAGER(c));
        }
        else
        {   setskippager(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateHidden])
    {   
        if(toggle)
        {   sethidden(c, !ISHIDDEN(c));
        }
        else
        {   sethidden(c, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateFocused])
    {
        if(toggle)
        {   SETFLAG(c->ewmhflags, WStateFlagFocused, !ISFOCUSED(c));
        }
        else
        {   SETFLAG(c->ewmhflags, WStateFlagFocused, add_remove_toggle);
        }
    }
    else if (state == netatom[NetWMStateShaded])
    {
        if(toggle)
        {   setshaded(c, !ISSHADED(c));
        }
        else
        {   setshaded(c, add_remove_toggle);
        }
    }
    else
    {   char *name = GetAtomNameQuick(_wm.dpy, state);
        if(name)
        {   Debug("Atom type: %s", name);
        }
        else
        {   Debug0("Could not find type.");
        }
        free(name);
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
        {   setwtypedesktop(c, !ISDESKTOP(c));
        }
        else
        {   setwtypedesktop(c, add_remove_toggle);
        }
        /* TODO */
    }
    else if (wtype == netatom[NetWMWindowTypeDock])
    {
        if(toggle)
        {   setwtypedock(c, !ISDOCK(c));
        }
        else
        {   setwtypedock(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeToolbar])
    {   
        if(toggle)
        {   setwtypetoolbar(c, !ISTOOLBAR(c));
        }
        else
        {   setwtypetoolbar(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeMenu])
    {
        if(toggle)
        {   setwtypemenu(c, !ISMENU(c));
        }
        else
        {   setwtypemenu(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeUtility])
    {
        if(toggle)
        {   setwtypeutility(c, !ISUTILITY(c));
        }
        else
        {   setwtypeutility(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeSplash])
    {
        if(toggle)
        {   setwtypesplash(c, !ISSPLASH(c));
        }
        else
        {   setwtypesplash(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDialog])
    {   
        if(toggle)
        {   setwtypedialog(c, !ISDIALOG(c));
        }
        else
        {   setwtypedialog(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDropdownMenu])
    {   
        if(toggle)
        {   setwtypedropdownmenu(c, !ISDROPDOWNMENU(c));
        }
        else
        {   setwtypedropdownmenu(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypePopupMenu])
    {
        if(toggle)
        {   setwtypepopupmenu(c, !ISPOPUPMENU(c));
        }
        else
        {   setwtypepopupmenu(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeTooltip])
    {
        if(toggle)
        {   setwtypetooltip(c, !ISTOOLTIP(c));
        }
        else
        {   setwtypetooltip(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeNotification])
    { 
        if(toggle)
        {   setwtypenotification(c, !ISNOTIFICATION(c));
        }
        else
        {   setwtypenotification(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeCombo])
    {
        if(toggle)
        {   setwtypecombo(c, !ISCOMBO(c));
        }
        else
        {   setwtypecombo(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeDnd])
    {
        if(toggle)
        {   setwtypednd(c, !ISDND(c));
        }
        else
        {   setwtypednd(c, add_remove_toggle);
        }
    }
    else if (wtype == netatom[NetWMWindowTypeNormal])
    {
        if(toggle)
        {   setwtypenormal(c, !ISNORMAL(c));
        }
        else
        {   setwtypenormal(c, add_remove_toggle);   
        }
    }
    else
    {   
        char *name = GetAtomNameQuick(_wm.dpy, wtype);
        if(name)
        {   Debug("Atom type: %s", name);
        }
        else
        {   Debug0("Could not find type.");
        }
        free(name);
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
            SETFLAG(c->ewmhflags, WStateFlagDemandAttention, !!(wmh->flags & XCB_WM_HINT_URGENCY));
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
                    Debug("Window Specified is Widthdrawn? %d", c->win);
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
    Client *c = getclienthash(win);

    if(c)
    {   return c;
    }

    /* malloc can fail some times so linear prob for client (unlikely), or it just doesnt exist. */
    Monitor *m;
    Desktop *desk;
    for(m = _wm.mons; m; m = nextmonitor(m))
    {
        for(desk = m->desktops; desk; desk = nextdesktop(desk))
        {
            for(c = startclient(desk); c; c = nextclient(c))
            {
                if(c->win == win)
                {   
                    Debug0("Found non hashed client.");
                    Debug0("Hashing...");
                    /* try and re-add it to the hasmap */
                    addclienthash(c);
                    return c;
                }
            }
        }
    }
    return NULL;
}
