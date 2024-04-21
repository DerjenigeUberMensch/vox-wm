#include "xcb_trl.h"
#include "events.h"
#include "util.h"
#include "config.h"
#include "keybinds.h"
#include "dwm.h"

extern WM _wm;
extern CFG _cfg;
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];

extern void xerror(XCBDisplay *display, XCBGenericError *error);

void (*handler[XCBLASTEvent]) (XCBGenericEvent *) = 
{
    /* Keyboard */
    [XCB_KEY_PRESS] = keypress,
    [XCB_KEY_RELEASE] = keyrelease,
    /* Pointer */
    [XCB_BUTTON_PRESS] = buttonpress,
    [XCB_BUTTON_RELEASE] = buttonrelease,
    [XCB_MOTION_NOTIFY] = motionnotify,
    /* Win Crossing */
    [XCB_ENTER_NOTIFY] = enternotify,
    [XCB_LEAVE_NOTIFY] = leavenotify,
    /* Input focus */
    [XCB_FOCUS_IN] = focusin,
    [XCB_FOCUS_OUT] = focusout,
    /* keymap state notification */
    [XCB_KEYMAP_NOTIFY] = keymapnotify,
    /* Exposure */
    [XCB_EXPOSE] = expose,
    [XCB_GRAPHICS_EXPOSURE] = graphicsexpose,
    [XCB_NO_EXPOSURE] = noexpose,
    /* structure control */
    [XCB_CONFIGURE_REQUEST] = configurerequest,
    [XCB_CIRCULATE_REQUEST] = circulaterequest,
    [XCB_MAP_REQUEST] = maprequest,
    [XCB_RESIZE_REQUEST] = resizerequest,

    /* window state notify */
    [XCB_CIRCULATE_NOTIFY] = circulatenotify,
    [XCB_CONFIGURE_NOTIFY] = configurenotify,
    [XCB_CREATE_NOTIFY] = createnotify,
    [XCB_DESTROY_NOTIFY] = destroynotify,
    [XCB_GRAVITY_NOTIFY] = gravitynotify,
    [XCB_MAP_NOTIFY] = mapnotify,
    [XCB_MAPPING_NOTIFY] = mappingnotify,
    [XCB_UNMAP_NOTIFY] = unmapnotify,
    [XCB_VISIBILITY_NOTIFY] = visibilitynotify,
    [XCB_REPARENT_NOTIFY] = reparentnotify,
    /* colour map state notify */
    [XCB_COLORMAP_NOTIFY] = colormapnotify,
    /* client communication */
    [XCB_CLIENT_MESSAGE] = clientmessage,
    [XCB_PROPERTY_NOTIFY] = propertynotify,
    [XCB_SELECTION_CLEAR] = selectionclear,
    [XCB_SELECTION_NOTIFY] = selectionnotify,
    [XCB_SELECTION_REQUEST] = selectionrequest,
    [XCB_GE_GENERIC] = genericevent,
    [XCB_NONE] = errorhandler,
};

void
keypress(XCBGenericEvent *event)
{
    XCBKeyPressEvent *ev = (XCBKeyPressEvent *)event;
    const i16 rootx             = ev->root_x;
    const i16 rooty             = ev->root_y;
    const i16 eventx            = ev->event_x;
    const i16 eventy            = ev->event_y;
    const u16 state             = ev->state;
    const XCBWindow eventroot   = ev->root;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow eventchild  = ev->child;
    const u8 samescreen         = ev->same_screen;
    const XCBKeyCode keydetail  = ev->detail;
    const XCBTimestamp tim      = ev->time;

    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)samescreen;
    (void)tim;

    const i32 cleanstate = CLEANMASK(state);
    /* ONLY use lowercase cause we dont know how to handle anything else */
    const XCBKeysym sym = XCBKeySymbolsGetKeySym(_wm.syms, keydetail, 0);
    /* Only use upercase cause we dont know how to handle anything else
     * sym = XCBKeySymbolsGetKeySym(_wm.syms,  keydetail, 0);
     */
    /* This Could work MAYBE allowing for upercase and lowercase Keybinds However that would complicate things due to our ability to mask Shift
     * sym = XCBKeySymbolsGetKeySym(_wm.syms, keydetail, cleanstate); 
     */
    DEBUG("%d", sym);
    int i;
    u8 sync = 0;
    for(i = 0; i < LENGTH(keys); ++i)
    {
        if(keys[i].type == XCB_KEY_PRESS)
        {
            if (sym == keys[i].keysym
                    && CLEANMASK(keys[i].mod) == cleanstate
                    && keys[i].func) 
            {   
                keys[i].func(&(keys[i].arg));
                sync = 1;
                break;
            }
        }
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
keyrelease(XCBGenericEvent *event)
{
    XCBKeyReleaseEvent *ev = (XCBKeyReleaseEvent *)event;
    const i16 rootx             = ev->root_x;
    const i16 rooty             = ev->root_y;
    const i16 eventx            = ev->event_x;
    const i16 eventy            = ev->event_y;
    const u16 state             = ev->state;
    const XCBWindow eventroot   = ev->root;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow eventchild  = ev->child;
    const u8 samescreen         = ev->same_screen;
    const XCBKeyCode keydetail  = ev->detail;
    const XCBTimestamp tim      = ev->time;

    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)samescreen;
    (void)tim;


    const i32 cleanstate = CLEANMASK(state);
    /* ONLY use lowercase cause we dont know how to handle anything else */
    const XCBKeysym sym = XCBKeySymbolsGetKeySym(_wm.syms, keydetail, 0);
    /* Only use upercase cause we dont know how to handle anything else
     * sym = XCBKeySymbolsGetKeySym(_wm.syms,  keydetail, 0);
     */
    /* This Could work MAYBE allowing for upercase and lowercase Keybinds However that would complicate things due to our ability to mask Shift
     * sym = XCBKeySymbolsGetKeySym(_wm.syms, keydetail, cleanstate); 
     */
    int i;
    u8 sync = 0;
    for(i = 0; i < LENGTH(keys); ++i)
    {
        if(keys[i].type == XCB_KEY_RELEASE)
        {
            if (sym == keys[i].keysym
                    && CLEANMASK(keys[i].mod) == cleanstate
                    && keys[i].func) 
            {   
                keys[i].func(&(keys[i].arg));
                sync = 1;
                break;
            }
        }
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
buttonpress(XCBGenericEvent *event)
{
    XCBButtonPressEvent *ev = (XCBButtonPressEvent *)event;
    const i16 rootx             = ev->root_x;
    const i16 rooty             = ev->root_y;
    const i16 eventx            = ev->event_x;
    const i16 eventy            = ev->event_y;
    const u16 state             = ev->state;
    const XCBWindow eventroot   = ev->root;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow eventchild  = ev->child;
    const u8 samescreen         = ev->same_screen;
    const XCBKeyCode keydetail  = ev->detail;
    const XCBTimestamp tim      = ev->time;


    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)eventroot;
    (void)eventchild;
    (void)samescreen;
    (void)tim;


    const i32 cleanstate = CLEANMASK(state);

    u8 sync = 0;
    Monitor *m = NULL;
    /* focus monitor if necessary */
    if ((m = wintomon(eventroot)))
    {
        if(m != _wm.selmon)
        {
            unfocus(_wm.selmon->desksel->sel, 1);
            _wm.selmon = m;
            focus(NULL);
            sync = 1;
        }
    }

    Client *c;
    if((c = wintoclient(eventwin)))
    {   
        focus(c);
        XCBAllowEvents(_wm.dpy, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
        sync = 1;
    }
    int i;
    for(i = 0; i < LENGTH(buttons); ++i)
    {   
        if(buttons[i].type == XCB_BUTTON_PRESS
            && buttons[i].func
            && buttons[i].button == keydetail
            && CLEANMASK(buttons[i].mask) == cleanstate)
        {
            Arg arg;
            arg.v = ev;
            buttons[i].func(&arg);
            sync = 1;
            DEBUG("%d", buttons[i].button);
            break;
        }
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
    DEBUG("ButtonPress: (x: %d, y: %d)", rootx, rooty);
}

void
buttonrelease(XCBGenericEvent *event)
{
    XCBButtonReleaseEvent *ev = (XCBButtonReleaseEvent *)event;
    const i16 rootx             = ev->root_x;
    const i16 rooty             = ev->root_y;
    const i16 eventx            = ev->event_x;
    const i16 eventy            = ev->event_y;
    const u16 state             = ev->state;
    const XCBWindow eventroot   = ev->root;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow eventchild  = ev->child;
    const u8 samescreen         = ev->same_screen;
    const XCBKeyCode keydetail  = ev->detail;
    const XCBTimestamp tim      = ev->time;


    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)samescreen;
    (void)tim;

    const i32 cleanstate = CLEANMASK(state);

    u8 sync = 0;

    i16 i;
    for(i = 0; i < LENGTH(buttons); ++i)
    {   
        if(buttons[i].type == XCB_BUTTON_RELEASE
            && buttons[i].func
            && buttons[i].button == keydetail
            && CLEANMASK(buttons[i].mask) == cleanstate)
        {
            Arg arg;
            arg.v = ev;
            buttons[i].func(&arg);
            sync = 1;
            DEBUG("%d", buttons[i].button);
            break;
        }
    }
    
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
motionnotify(XCBGenericEvent *event)
{
    XCBMotionNotifyEvent *ev = (XCBMotionNotifyEvent *)event;
    const i16 rootx             = ev->root_x;
    const i16 rooty             = ev->root_y;
    const i16 eventx            = ev->event_x;
    const i16 eventy            = ev->event_y;
    const u16 state             = ev->state;
    const XCBWindow eventroot   = ev->root;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow eventchild  = ev->child;
    const u8 samescreen         = ev->same_screen;
    const XCBKeyCode keydetail  = ev->detail;
    const XCBTimestamp tim      = ev->time;


    (void)eventx;
    (void)eventy;
    (void)eventroot;
    (void)state;
    (void)eventchild;
    (void)samescreen;
    (void)keydetail;
    (void)tim;


    /* due to the mouse being able to move a ton we want to limit the cycles burnt for non root events */
    if(eventwin != _wm.root)
    {   return;
    }

    u8 sync = 0;
    static Monitor *mon = NULL;
    Monitor *m;

    //DEBUG("(x: %d, y: %d)", rootx, rooty);
    //DEBUG("(w: %d, h: %d)", XCBDisplayWidth(_wm.dpy, _wm.screen), XCBDisplayHeight(_wm.dpy, _wm.screen));
    if((m = recttomon(rootx, rooty, 1, 1)) != mon && mon)
    {
        Client *c = _wm.selmon->desksel->sel;
        if(c)
        {   unfocus(c, 1);
        }
        _wm.selmon = m;
        focus(NULL);
        sync = 1;
    }
    mon = m;

    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
enternotify(XCBGenericEvent *event)
{
    XCBEnterNotifyEvent *ev = (XCBEnterNotifyEvent *)event;
    const uint8_t detail    = ev->detail;
    const XCBTimestamp tim  = ev->time;
    const XCBWindow eventroot = ev->root;
    const XCBWindow eventwin = ev->event;
    const XCBWindow eventchild = ev->child;
    const int16_t rootx = ev->root_x;
    const int16_t rooty = ev->root_y;
    const int16_t eventx = ev->event_x;
    const int16_t eventy = ev->event_y;
    const uint16_t state = ev->state;
    const uint8_t mode   = ev->mode;
    const uint8_t samescreenfocus = ev->same_screen_focus;



    (void)detail;
    (void)tim;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)state;
    (void)mode;
    (void)samescreenfocus;

    if(!_cfg.hoverfocus) return;


    /* hover focus */


    Client *c;
    Monitor *m;
    u8 sync = 0;

    if((mode != XCB_NOTIFY_MODE_NORMAL || detail == XCB_NOTIFY_DETAIL_INFERIOR) && eventwin != _wm.root)
    {   return;
    }

    c = wintoclient(eventwin);
    m = c ? c->desktop->mon : wintomon(eventwin);

    if(m != _wm.selmon)
    {
        unfocus(_wm.selmon->desksel->sel, 1);
        _wm.selmon = m;
        sync = 1;
    }
    else if(!c || c == _wm.selmon->desksel->sel)
    {   return;
    }
    focus(c);
    sync = 1;
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
leavenotify(XCBGenericEvent *event)
{
    XCBLeaveNotifyEvent *ev = (XCBLeaveNotifyEvent *)event;
    const uint8_t detail    = ev->detail;
    const XCBTimestamp tim  = ev->time;
    const XCBWindow eventroot = ev->root;
    const XCBWindow eventwin = ev->event;
    const XCBWindow eventchild = ev->child;
    const int16_t rootx = ev->root_x;
    const int16_t rooty = ev->root_y;
    const int16_t eventx = ev->event_x;
    const int16_t eventy = ev->event_y;
    const uint16_t state = ev->state;
    const uint8_t mode   = ev->mode;
    const uint8_t samescreenfocus = ev->same_screen_focus;

    (void)detail;
    (void)tim;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)state;
    (void)mode;
    (void)samescreenfocus;
    
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XCBGenericEvent *event)
{
    XCBFocusInEvent *ev = (XCBFocusInEvent *)event;
    const u8 detail = ev->detail;
    const XCBWindow eventwin = ev->event;
    const u8 mode = ev->mode;

    (void)detail;
    (void)mode;

    u8 sync = 0;

    if(_wm.selmon->desksel->sel && eventwin != _wm.selmon->desksel->sel->win)
    {   
        setfocus(_wm.selmon->desksel->sel);
        sync = 1;
    }

    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
focusout(XCBGenericEvent *event)
{
    XCBFocusOutEvent *ev = (XCBFocusOutEvent *)event;
    const u8 detail = ev->detail;
    const XCBWindow eventwin = ev->event;
    const u8 mode = ev->mode;

    (void)detail;
    (void)eventwin;
    (void)mode;
}

void
keymapnotify(XCBGenericEvent *event)
{
    XCBKeymapNotifyEvent *ev = (XCBKeymapNotifyEvent *)event;
    u8 *eventkeys   = ev->keys;        /* DONOT FREE */

    (void)eventkeys;
}

void
expose(XCBGenericEvent *event)
{
    XCBExposeEvent *ev = (XCBExposeEvent *)event;
    const XCBWindow win     = ev->window;
    const i16 x             = ev->x;
    const i16 y             = ev->y;
    const u16 w             = ev->width;
    const u16 h             = ev->height;
    const u16 count         = ev->count;


    (void)x;
    (void)y;
    (void)w;
    (void)h;

    Monitor *m = wintomon(win);
    if(count == 0 && m)
    {   /* redrawbar */
    }
}

void
graphicsexpose(XCBGenericEvent *event)
{
    XCBGraphicsExposeEvent *ev = (XCBGraphicsExposeEvent *)event;
    const i16 x                 = ev->x;
    const i16 y                 = ev->y;
    const u16 w                 = ev->width;
    const u16 h                 = ev->height;
    const u16 count             = ev->count;
    const XCBDrawable drawable  = ev->drawable;
    const u8 majoropcode        = ev->major_opcode;
    const u16 minoropcode       = ev->minor_opcode;


    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)count;
    (void)drawable;
    (void)majoropcode;
    (void)minoropcode;
}

void
noexpose(XCBGenericEvent *event)
{
    XCBExposeEvent *ev = (XCBExposeEvent *)event;
    const i16 x                 = ev->x;
    const i16 y                 = ev->y;
    const u16 w                 = ev->width;
    const u16 h                 = ev->height;
    const u16 count             = ev->count;
    const XCBWindow win         = ev->window;


    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)count;
    (void)win;
}

void
circulaterequest(XCBGenericEvent *event)
{
    XCBCirculateRequestEvent *ev = (XCBCirculateRequestEvent *)event;
    const XCBWindow win         = ev->window;
    const XCBWindow eventwin    = ev->event;
    const u8 place              = ev->place;

    (void)win;
    (void)eventwin;
    (void)place;
}

void
configurerequest(XCBGenericEvent *event)
{   
    XCBConfigureRequestEvent *ev = (XCBConfigureRequestEvent *)event;
    const i16 x     = ev->x;
    const i16 y     = ev->y;
    const u16 w     = ev->width;
    const u16 h     = ev->height;
    const u16 bw    = ev->border_width;
    const u16 mask  = ev->value_mask;
    const u8  stack = ev->stack_mode;
    const XCBWindow win     = ev->window;
    const XCBWindow parent  = ev->parent;
    const XCBWindow sibling = ev->sibling;


    (void)parent;

    Client *c;
    u8 sync = 0;
    if((c = wintoclient(win)))
    {
        const Monitor *m = c->desktop->mon;
        if(mask & XCB_CONFIG_WINDOW_BORDER_WIDTH)
        {                           /* Border width should NEVER be bigger than the screen */
            setborderwidth(c, bw);
        }
        if(mask & XCB_CONFIG_WINDOW_X)
        {
            c->oldx = c->x;
            c->x = m->mx + x;
        }
        if(mask & XCB_CONFIG_WINDOW_Y)
        {
            c->oldy = c->y;
            c->y = m->my + y;
        }
        if(mask & XCB_CONFIG_WINDOW_WIDTH)
        {
            c->oldw = c->w;
            c->w = w;
        }
        if(mask & XCB_CONFIG_WINDOW_HEIGHT)
        {
            c->oldh = c->h;
            c->h = h;
        }
        if(mask & XCB_CONFIG_WINDOW_SIBLING)
        {
            ASSUME(0);
            if(sibling != XCBNone)
            {   /* Ignore these requests we handle stack order */
            }
        }
        if(mask & XCB_CONFIG_WINDOW_STACK_MODE)
        {
            /* Ignore these requests we handle stack order */
            ASSUME(0);
            switch(stack)
            {
                case XCB_STACK_MODE_ABOVE: /* XRaiseAboveSibling(ev->above) */ break;
                case XCB_STACK_MODE_BELOW: /* XLowerBelowSibling(ev->above) */ break;
                case XCB_STACK_MODE_TOP_IF: /* XRaiseWindow(dpy, ev->window) */ break;
                case XCB_STACK_MODE_BOTTOM_IF:/* XLowerToBottomWindow(dpy, e->window)*/ break;
                case XCB_STACK_MODE_OPPOSITE: /* XFlipStackOrder(ev->above, ev->window)*/ break;
            }
        }
        if((c->x + c->w) > m->mx + m->mw && ISFLOATING(c))
        {   
            c->oldx = c->x;
            c->x = m->mx + ((m->mw >> 1) - (WIDTH(c) >> 1)); /* center in x direction */
        }
        if((c->y + c->h) > m->my + m->mh && ISFLOATING(c))
        {   
            c->oldy = c->y;
            c->y = m->my + ((m->mh >> 1) - (HEIGHT(c) >> 1)); /* center in y direction */
        }
        if(mask & (XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y) && !(mask & (XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT)))
        {    configure(c);
        }
        if(ISVISIBLE(c))
        {   XCBMoveResizeWindow(_wm.dpy, c->win, c->x, c->y, c->w, c->h);
        }
        sync = 1;
    }
    else
    {
        XCBWindowChanges wc;
        wc.x = x;
        wc.y = y;
        wc.width = w;
        wc.height = h;
        wc.border_width = bw;
        wc.sibling = sibling;
        wc.stack_mode = stack;
        /* some windows need to be mapped before configuring */
        XCBConfigureWindow(_wm.dpy, win, mask, &wc);
        sync = 1;
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
maprequest(XCBGenericEvent *event)
{
    XCBMapRequestEvent *ev  = (XCBMapRequestEvent *)event;
    const XCBWindow parent  = ev->parent;
    const XCBWindow win     = ev->window;

    (void)parent;

    u8 sync = 0;

    if(!wintoclient(win))
    {
        /* only sync if we successfully managed the window */   
        sync = !!manage(win);
    }

    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}
/* popup windows sometimes need this */
void
resizerequest(XCBGenericEvent *event)
{
    XCBResizeRequestEvent *ev = (XCBResizeRequestEvent *)event;
    const XCBWindow win = ev->window;
    const u16 w         = ev->width;
    const u16 h         = ev->height;

    Client *c;
    
    u8 sync = 0;

    if((c = wintoclient(win)))
    {   
        resize(c, c->x, c->y, w, h, 0);
        sync = 1;
    }
    else
    {   
        XCBResizeWindow(_wm.dpy, win, w, h);
        sync = 1;   /* we dont technically need to sync here but its just to catch up on somethings if we fucked up */
    }

    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
circulatenotify(XCBGenericEvent *event)
{
    XCBCirculateNotifyEvent *ev = (XCBCirculateNotifyEvent *)event;
    const u8 place                    = ev->place;
    const XCBWindow win               = ev->window;
    const XCBWindow eventwin          = ev->event;

    (void)place;
    (void)win;
    (void)eventwin;
}

/* These events are mostly just Info events of stuff that has happened already
 * so this tells that happened (x/y/w/h) and only tells that (AKA sends this event)
 * if we sucesfully did that action So we only really need to check root events here
 * cause this only occurs on sucesfull actions
 */
void
configurenotify(XCBGenericEvent *event)
{
    XCBConfigureNotifyEvent *ev = (XCBConfigureNotifyEvent *)event;
    const XCBWindow eventwin = ev->event;
    const XCBWindow win = ev->window;
    const XCBWindow abovesibling = ev->above_sibling;
    const i16 x = ev->x;
    const i16 y = ev->y;
    const u16 w = ev->width;
    const u16 h = ev->height;
    const u16 borderwidth = ev->border_width;
    const u8 overrideredirect = ev->override_redirect;

    (void)eventwin;
    (void)abovesibling;
    (void)x;
    (void)y;
    (void)borderwidth;

    u8 sync = 0;
    if(win == _wm.root)
    {
        u8 dirty;
        dirty = (_wm.sw != w || _wm.sh != h);
        
        _wm.sw = w;
        _wm.sh = h;


        DEBUG("(w: %d, h: %d)", w, h);
        if(updategeom() || dirty)
        {
            Monitor *m;
            /* update the bar */
            for(m = _wm.mons; m; m = nextmonitor(m))
            {
                if(m->bar->win)
                {   XCBMoveResizeWindow(_wm.dpy, m->bar->win, m->wx, m->bar->y, m->ww, m->bar->h);
                }
            }
            focus(NULL);
            arrangemons();
            sync = 1;
        }
    }
    else if(overrideredirect)
    {
        Client *c;
        Bar *b;
        if((c = wintoclient(win)))
        {
            unmanage(c, 0);
            sync = 1;
        }
        else if((b = wintobar(win, 0)))
        {   
            unmanagebar(b);
            sync = 1;
        }
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
createnotify(XCBGenericEvent *event)
{
    XCBCreateNotifyEvent *ev = (XCBCreateNotifyEvent *)event;
    const u8 overrideredirect   = ev->override_redirect;
    const XCBWindow win         = ev->window;
    const XCBWindow parentwin   = ev->parent;
    const i16 x                 = ev->x;
    const i16 y                 = ev->y;
    const u16 w                 = ev->width;
    const u16 h                 = ev->height;
    const u16 bw                = ev->border_width;


    (void)overrideredirect;
    (void)win;
    (void)parentwin;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)bw;
}

void
destroynotify(XCBGenericEvent *event)
{
    XCBDestroyNotifyEvent *ev = (XCBDestroyNotifyEvent *)event;
    const XCBWindow win         = ev->window;
    const XCBWindow eventwin    = ev->event;        /* The Event win is the window that sent the message */

    (void)eventwin;

    Client *c = NULL;
    Bar *b = NULL;
    u8 sync = 0;
    /* destroyed windows no longer need to be managed */
    if((c = wintoclient(win)))
    {   
        unmanage(c, 1);
        sync = 1;
    }
    else if((b = wintobar(win, 0)))
    {
        unmanagebar(b);
        sync = 1;
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
gravitynotify(XCBGenericEvent *event)
{
    XCBGravityNotifyEvent *ev = (XCBGravityNotifyEvent *)event;
    const XCBWindow eventwin          = ev->event;
    const XCBWindow win               = ev->window;
    const i16 x                       = ev->x;
    const i16 y                       = ev->y;

    (void)eventwin;
    (void)win;
    (void)x;
    (void)y;
}

void
mapnotify(XCBGenericEvent *event)
{
    XCBMapNotifyEvent *ev = (XCBMapNotifyEvent *)event;
    const XCBWindow win             = ev->window;
    const XCBWindow eventwin        = ev->event;
    const uint8_t override_redirect = ev->override_redirect;

    (void)win;
    (void)eventwin;
    (void)override_redirect;
}

void
mappingnotify(XCBGenericEvent *event)
{
    XCBMappingNotifyEvent *ev = (XCBMappingNotifyEvent *)event;
    const XCBKeyCode first_keycode = ev->first_keycode;
    const uint8_t count            = ev->count;
    const uint8_t request          = ev->request;

    (void)count;
    (void)first_keycode;

    XCBRefreshKeyboardMapping(_wm.syms, ev);
    /* update the mask */
    updatenumlockmask();
    if(request == XCB_MAPPING_KEYBOARD)
    {   grabkeys();
    }
    else if(request == XCB_MAPPING_POINTER)
    {
    }
    XCBFlush(_wm.dpy);
}

void
unmapnotify(XCBGenericEvent *event)
{
    XCBUnMapNotifyEvent *ev = (XCBUnMapNotifyEvent *)event;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow win         = ev->window;
    const uint8_t isconfigure   = ev->from_configure;

    (void)eventwin;
    (void)isconfigure;

    Client *c;
    Bar *bar;
    u8 sync = 0;
    if((c = wintoclient(win)))
    {   
        unmanage(c, 0);
        sync = 1;
    }
    else if((bar = wintobar(win, 0)))
    {
        unmanagebar(bar);
        sync = 1;
    }

    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
visibilitynotify(XCBGenericEvent *event)
{
    XCBVisibilityNotifyEvent *ev = (XCBVisibilityNotifyEvent *)event;
    const XCBWindow win         = ev->window;
    const u8 state              = ev->state;

    (void)win;
    (void)state;
}

void
reparentnotify(XCBGenericEvent *event)
{
    XCBReparentNotifyEvent *ev = (XCBReparentNotifyEvent *)event;
    const XCBWindow parent      = ev->parent;
    const XCBWindow win         = ev->window;
    const XCBWindow eventwin    = ev->event;
    const i16 x                 = ev->x;
    const i16 y                 = ev->y;
    const u8 override_redirect  = ev->override_redirect;

    (void)parent;
    (void)win;
    (void)eventwin;
    (void)x;
    (void)y;
    (void)override_redirect;
}

void
colormapnotify(XCBGenericEvent *event)
{
    XCBColormapNotifyEvent *ev = (XCBColormapNotifyEvent *)event;
    const XCBWindow win         = ev->window;
    const u8 state              = ev->state;
    const XCBColormap colormap  = ev->colormap;
    const u8 new                = ev->_new;

    (void)win;
    (void)state;
    (void)colormap;
    (void)new;
}

/* TODO */
void
clientmessage(XCBGenericEvent *event)
{
    XCBClientMessageEvent *ev = (XCBClientMessageEvent *)event;
    const XCBWindow win             = ev->window;
    const XCBAtom atom              = ev->type;
    const u8 format                 = ev->format;
    const XCBClientMessageData data = ev->data;     /* union "same" as xlib data8 -> b[20] data16 -> s[10] data32 = l[5] */


    (void)format;

    /* move resize */
    #define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
    #define _NET_WM_MOVERESIZE_SIZE_TOP          1
    #define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
    #define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
    #define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
    #define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
    #define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
    #define _NET_WM_MOVERESIZE_SIZE_LEFT         7
    #define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
    #define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
    #define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10   /* move via keyboard */
    #define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

    /* These cover most of the important message's */
    /*
     * _NET_NUMBER_OF_DESKTOPS
     * _NET_DESKTOP_GEOMETRY
     * _NET_DESKTOP_VIEWPORT
     * _NET_CURRENT_DESKTOP
     * _NET_ACTIVE_WINDO
     * _NET_SHOWING_DESKTOP
     * _NET_CLOSE_WINDOW
     * _NET_MOVERESIZE_WINDOW
     * _NET_WM_MOVERESIZE
     * _NET_RESTACK_WINDOW
     * _NET_REQUEST_FRAME_EXTENTS
     * _NET_WM_DESKTOP
     * 
     *
     * _NET_WM_STATE
     * _NET_WM_WINDOW_TYPE
     * 
     *
     * _NET_WM_FULLSCREEN_MONITORS
     * 
     * subtypes:
     *
     * WM_PROTOCOLS:
     *      data.l[0]       _NET_WM_PING
     *      data.l[0]       _NET_WM_SYNC_REQUEST
     */

    Client *c = wintoclient(win);
    if(c)
    {
        /* long data is often used and anything else is just padding */
        const i32 l0 = data.data32[0];
        const i32 l1 = data.data32[1];
        const i32 l2 = data.data32[2];
        const i32 l3 = data.data32[3];
        const i32 l4 = data.data32[4];
        if(atom == netatom[NetWMState])
        {
            const u8 action = l0;   /* remove: 0 
                                     * add: 1 
                                     * toggle: 2 
                                     */
            const XCBAtom prop1 = l1;
            const XCBAtom prop2 = l2;
            updatewindowstate(c, prop1, action);
            updatewindowstate(c, prop2, action);
        }
        else if(atom == netatom[NetActiveWindow])
        {
            if(c->desktop && c->desktop->sel != c && !ISURGENT(c))
            {   seturgent(c, 1);
            }
        }
        else if(atom == netatom[NetCloseWindow])
        {   
            killclient(c, Graceful);
        }
        else if(atom == netatom[NetMoveResizeWindow])
        {
            const u32 gravity = l0;
            /* 64bit to cover bounds checks */
            i32 x = l1;
            i32 y = l2;
            i32 w = l3;
            i32 h = l4;
            i32 bw = c->bw;

            applysizechecks(c->desktop->mon, &x, &y, &w, &h, &(bw));
    
            i16 cleanx = x;
            i16 cleany = y;
            const i16 cleanw = w;
            const i16 cleanh = h;

            applygravity(gravity, &cleanx, &cleany, cleanw, cleanh, c->bw);
            resize(c, cleanx, cleany, cleanw, cleanh, 0);
        }
        else if(atom == netatom[NetMoveResize])
        {
            const int netwmstate = l2;
            /* TODO */
            switch(netwmstate)
            {
                case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
                case _NET_WM_MOVERESIZE_SIZE_TOP:
                case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
                case _NET_WM_MOVERESIZE_SIZE_RIGHT:
                case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
                case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
                case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
                case _NET_WM_MOVERESIZE_SIZE_LEFT:
                    break;
                case _NET_WM_MOVERESIZE_MOVE:
                    break;
                case _NET_WM_MOVERESIZE_SIZE_KEYBOARD: 
                    break;
                case _NET_WM_MOVERESIZE_MOVE_KEYBOARD: 
                    break;
                case _NET_WM_MOVERESIZE_CANCEL: 
                    break;
            }
        }
        else if(atom == netatom[NetMoveResizeWindow])
        {
            i32 gravity = l0;
            i16 x = l1;
            i16 y = l2;
            const u16 w = l3;
            const u16 h = l4;
            applygravity(gravity, &x, &y, w, h, c->bw);
            resize(c, x, y, w, h, 0);
        }
        else if(atom == netatom[NetRestackWindow])
        {   /* TODO */
        }
        else if(atom == netatom[NetRequestFrameExtents])
        {   /* TODO */
        }
        else if (atom == netatom[NetNumberOfDesktops])
        {   /* ignore */
        }
        else if (atom == netatom[NetDesktopGeometry])
        {   /* ignore */
        }
        else if (atom == netatom[NetDesktopViewport])
        {   /* TODO */
        }
        else if (atom == netatom[NetCurrentDesktop])
        {   
            u32 target = l0;
            Monitor *m = c->desktop->mon;
            if(m)
            {
                Desktop *desk;
                u32 i = 0;
                for(desk = m->desktops; desk && i != target; desk = nextdesktop(desk), ++i);
                if(desk)
                {   setclientdesktop(c, desk);
                }
                else
                {   DEBUG0("Desktop was not in range defaulting to no desktop change.");
                }
            }
        }
        else if (atom == netatom[NetShowingDesktop])
        {   /* TODO */
        }
        else if (atom == netatom[NetWMDesktop])
        {
            /* refer: https://specifications.freedesktop.org/wm-spec/latest/ _NET_WM_DESKTOP */
            if(checksticky(l0))
            {   
                setsticky(c, 1);
                return;
            }
        }
        else if (atom == netatom[WMProtocols])
        {   /* Protocol handler */
        }
        else if (atom == netatom[NetWMFullscreenMonitors])
        {   /* TODO */
        }
    }
}

void
propertynotify(XCBGenericEvent *event)
{
    XCBPropertyNotifyEvent *ev = (XCBPropertyNotifyEvent *)event;
    const XCBAtom atom          = ev->atom;
    const XCBWindow win         = ev->window;
    const XCBTimestamp timestamp= ev->time;
    const u16 state             = ev->state;


    (void)timestamp;

    Client *c = NULL;
    u8 sync = 0;
    if(win == _wm.root && atom == XCB_ATOM_WM_NAME)
    {   /* updatestatus */
    }

    if(state == XCB_PROPERTY_DELETE)
    {   return;
    }

    if(_wm.selmon->bar && _wm.selmon->bar->win == win)
    {
        /* probably one of the _NET_WM_STRUT's */
        if(atom == netatom[NetWMStrutPartial] || atom == netatom[NetWMStrut])
        {   
            updatebargeom(_wm.selmon);
            updatebarpos(_wm.selmon);
        }
        else
        {
            DEBUG0("Unknown atom prop.");
        }
    }

    if((c = wintoclient(win)))
    {   
        XCBCookie cookie;
        XCBWMHints *wmh;
        XCBWindow trans;
        uint8_t transstatus = 0;
        Client *tmp = NULL;
        switch(atom)
        {
            case XCB_ATOM_WM_TRANSIENT_FOR:
                cookie = XCBGetTransientForHintCookie(_wm.dpy, win);
                transstatus = XCBGetTransientForHintReply(_wm.dpy, cookie, &trans);
                if(transstatus)
                {
                    if((tmp = wintoclient(trans)))
                    {   
                        setwtypedialog(tmp, 1);
                        sync = 1;
                    }
                }
                break;
            case XCB_ATOM_WM_NORMAL_HINTS:
                break;
            case XCB_ATOM_WM_HINTS:
                /* TODO This can be slow */
                cookie = XCBGetWMHintsCookie(_wm.dpy, c->win);
                wmh = XCBGetWMHintsReply(_wm.dpy, cookie);
                updatewmhints(c, wmh);
                sync = 1;
                free(wmh);
                break;
            default:
                  break;
        }
    }
    if(sync)
    {   XCBFlush(_wm.dpy);
    }
}

void
selectionclear(XCBGenericEvent *event)
{
    XCBSelectionClearEvent *ev = (XCBSelectionClearEvent *)event;
    const XCBWindow owner       = ev->owner;
    const XCBAtom selection     = ev->selection;
    const XCBTimestamp tim      = ev->time;

    (void)owner;
    (void)selection;
    (void)tim;
}

void
selectionnotify(XCBGenericEvent *event)
{
    XCBSelectionNotifyEvent *ev     = (XCBSelectionNotifyEvent *)event;
    const XCBWindow requestor       = ev->requestor;
    const XCBAtom property          = ev->property;
    const XCBAtom target            = ev->target;
    const XCBAtom selection         = ev->selection;
    const XCBTimestamp tim          = ev->time;

    (void)requestor;
    (void)property;
    (void)target;
    (void)selection;
    (void)tim;
}

void
selectionrequest(XCBGenericEvent *event)
{
    XCBSelectionRequestEvent *ev = (XCBSelectionRequestEvent *)event;
    const XCBWindow owner       = ev->owner;
    const XCBWindow requestor   = ev->requestor;
    const XCBAtom property      = ev->property;
    const XCBAtom target        = ev->target;
    const XCBAtom selection     = ev->selection;
    const XCBTimestamp tim      = ev->time;

    (void)owner;
    (void)requestor;
    (void)property;
    (void)target;
    (void)selection;
    (void)tim;
}



void
genericevent(XCBGenericEvent *event)
{
}

void
errorhandler(XCBGenericEvent *event)
{   xerror(_wm.dpy, (XCBGenericError *)event);
}
