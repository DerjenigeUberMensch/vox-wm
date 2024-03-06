#include "xcb_trl.h"
#include "events.h"
#include "util.h"
#include "config.h"
#include "dwm.h"

extern WM *_wm;

extern void xerror(XCBDisplay *display, XCBGenericError *error);

void (*handler[LASTEvent]) (XCBGenericEvent *) = 
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


    const i32 cleanstate = CLEANMASK(state);

    /* ONLY use lowercase cause we dont know how to handle anything else */
    const XCBKeysym sym = XCBKeySymbolsGetKeySym(_wm->syms, keydetail, 0);
    /* Only use upercase cause we dont know how to handle anything else
     * sym = XCBKeySymbolsGetKeySym(_wm->syms,  keydetail, 0);
     */
    /* This Could work MAYBE allowing for upercase and lowercase Keybinds However that would complicate things due to our ability to mask Shift
     * sym = XCBKeySymbolsGetKeySym(_wm->syms, keydetail, cleanstate); 
     */
    int i;
    for(i = 0; i < LENGTH(keys); ++i)
    {
        if(keys[i].type == XCB_KEY_PRESS)
        {
            if (sym == keys[i].keysym
                    && CLEANMASK(keys[i].mod) == cleanstate
                    && keys[i].func) 
            {   keys[i].func(&(keys[i].arg));
                return;
            }
        }
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

    const i32 cleanstate = CLEANMASK(state);
    /* ONLY use lowercase cause we dont know how to handle anything else */
    const XCBKeysym sym = XCBKeySymbolsGetKeySym(_wm->syms, keydetail, 0);
    /* Only use upercase cause we dont know how to handle anything else
     * sym = XCBKeySymbolsGetKeySym(_wm->syms,  keydetail, 0);
     */
    /* This Could work MAYBE allowing for upercase and lowercase Keybinds However that would complicate things due to our ability to mask Shift
     * sym = XCBKeySymbolsGetKeySym(_wm->syms, keydetail, cleanstate); 
     */
    int i;
    for(i = 0; i < LENGTH(keys); ++i)
    {
        if(keys[i].type == XCB_KEY_RELEASE)
        {
            if (sym == keys[i].keysym
                    && CLEANMASK(keys[i].mod) == cleanstate
                    && keys[i].func) 
            {   keys[i].func(&(keys[i].arg));
                return;
            }
        }
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

    Monitor *m;
    /* focus monitor if necessary */
    if ((m = wintomon(eventwin)) && m != _wm->selmon)
    {
        unfocus(_wm->selmon->sel, 1);
        _wm->selmon = m;
        focus(NULL);
    }
    Client *c;
    if((c = wintoclient(eventwin)))
    {   
        if(m && m->sel != c)   
        {   
            if(c->mon != m)
            {   c->mon = m;
            }
            detachclient(c);
            attachclient(c);
            focus(c);
            if(ISFLOATING(c) || ISALWAYSONTOP(c))
            {   XCBRaiseWindow(_wm->dpy, c->win);
            }
            XCBAllowEvents(_wm->dpy, , XCB_TIME_CURRENT_TIME);
        }
    }

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
}

void
enternotify(XCBGenericEvent *event)
{
    XCBEnterNotifyEvent *ev = (XCBEnterNotifyEvent *)event;
}

void
leavenotify(XCBGenericEvent *event)
{
    XCBLeaveNotifyEvent *ev = (XCBLeaveNotifyEvent *)event;
}

void
focusin(XCBGenericEvent *event)
{
    XCBFocusInEvent *ev = (XCBFocusInEvent *)event;
    const u8 detail = ev->detail;
    const XCBWindow eventwin = ev->event;
    const u8 mode = ev->mode;
}

void
focusout(XCBGenericEvent *event)
{
    XCBFocusOutEvent *ev = (XCBFocusOutEvent *)event;
    const u8 detail = ev->detail;
    const XCBWindow eventwin = ev->event;
    const u8 mode = ev->mode;
}

void
keymapnotify(XCBGenericEvent *event)
{
    XCBKeymapNotifyEvent *ev = (XCBKeymapNotifyEvent *)event;
    u8 *keys = ev->keys;        /* DONOT FREE */
}

void
expose(XCBGenericEvent *event)
{
    XCBExposeEvent *ev = (XCBExposeEvent *)event;
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
}

void
circulaterequest(XCBGenericEvent *event)
{
    XCBCirculateRequestEvent *ev = (XCBCirculateRequestEvent *)event;
    const XCBWindow win         = ev->window;
    const XCBWindow eventwin    = ev->event;
    const u8 place              = ev->place;
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
}

void
maprequest(XCBGenericEvent *event)
{
    XCBMapRequestEvent *ev  = (XCBMapRequestEvent *)event;
    const XCBWindow parent  = ev->parent;
    const XCBWindow win     = ev->window;
    XCBMapWindow(_wm->dpy, win);
}
void
resizerequest(XCBGenericEvent *event)
{
    XCBResizeRequestEvent *ev = (XCBResizeRequestEvent *)event;
    const XCBWindow win = ev->window;
    const u16 w         = ev->width;
    const u16 h         = ev->height;
}

void
circulatenotify(XCBGenericEvent *event)
{
    XCBCirculateNotifyEvent *ev = (XCBCirculateNotifyEvent *)event;
}

void
configurenotify(XCBGenericEvent *event)
{
    XCBConfigureNotifyEvent *ev = (XCBConfigureNotifyEvent *)event;
}

void
createnotify(XCBGenericEvent *event)
{
    XCBCreateNotifyEvent *ev = (XCBCreateNotifyEvent *)event;
}

void
destroynotify(XCBGenericEvent *event)
{
    XCBDestroyNotifyEvent *ev = (XCBDestroyNotifyEvent *)event;
}

void
gravitynotify(XCBGenericEvent *event)
{
    XCBGravityNotifyEvent *ev = (XCBGravityNotifyEvent *)event;
}

void
mapnotify(XCBGenericEvent *event)
{
    XCBMapNotifyEvent *ev = (XCBMapNotifyEvent *)event;
}

void
mappingnotify(XCBGenericEvent *event)
{
    XCBMappingNotifyEvent *ev = (XCBMappingNotifyEvent *)event;
}

void
unmapnotify(XCBGenericEvent *event)
{
    XCBUnMapNotifyEvent *ev = (XCBUnMapNotifyEvent *)event;
}

void
visibilitynotify(XCBGenericEvent *event)
{
    XCBVisibilityNotifyEvent *ev = (XCBVisibilityNotifyEvent *)event;
}

void
reparentnotify(XCBGenericEvent *event)
{
    XCBReparentNotifyEvent *ev = (XCBReparentNotifyEvent *)event;
}

void
colormapnotify(XCBGenericEvent *event)
{
    XCBColormapNotifyEvent *ev = (XCBColormapNotifyEvent *)event;
}

void
clientmessage(XCBGenericEvent *event)
{
    XCBClientMessageEvent *ev = (XCBClientMessageEvent *)event;
    const XCBWindow win             = ev->window;
    const XCBAtom atom              = ev->type;
    const u8 format                 = ev->format;
    const XCBClientMessageData data = ev->data;     /* union "same" as xlib data8 -> b[20] data16 -> s[10] data32 = l[5] */
}

void
propertynotify(XCBGenericEvent *event)
{
    XCBPropertyNotifyEvent *ev = (XCBPropertyNotifyEvent *)event;
}

void
selectionclear(XCBGenericEvent *event)
{
    XCBSelectionClearEvent *ev = (XCBSelectionClearEvent *)event;
}

void
selectionnotify(XCBGenericEvent *event)
{
    XCBSelectionNotifyEvent *ev = (XCBSelectionNotifyEvent *)event;
}

void
selectionrequest(XCBGenericEvent *event)
{
    XCBSelectionRequestEvent *ev = (XCBSelectionRequestEvent *)event;
}


void
genericevent(XCBGenericEvent *event)
{   (void)event;
}

void
errorhandler(XCBGenericEvent *event)
{   xerror(_wm->dpy, (XCBGenericError *)event);
}
