#include "xcb_wmutil.h"
#include <stdint.h>
#include <stdio.h>

static int __boundcheck_dimentions(int32_t *x, int32_t *y, int32_t *w, int32_t *h);

static void
__apply_net_gravity(uint32_t gravity, int32_t *x, int32_t *y, int32_t w, int32_t h, int32_t bw)
{
    if(!gravity || !x || !y)
    {   return;
    }
    __boundcheck_dimentions(NULL, NULL, &w, &h);
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

/* return 0 on Success.
 * return -1 on failure.
 */
static int 
__boundcheck_dimentions(int32_t *x, int32_t *y, int32_t *w, int32_t *h)
{
    if(x)
    {
        int32_t nx = *x;

        /* i16 checks */
        if(nx > INT16_MAX)
        {   nx = INT16_MAX;
        }
        else if(nx < INT16_MIN)
        {   nx = INT16_MIN;
        }
        *x = nx;
    }
    if(y)
    {
        int32_t ny = *y;

        /* i16 checks */
        if(ny > INT16_MAX)
        {   ny = INT16_MAX;
        }
        else if(ny < INT16_MIN)
        {   ny = INT16_MIN;
        }
        *y = ny;
    }
    if(w)
    {
        int32_t nw = *w;
        /* min width */
        if(nw <= 0)
        {   nw = 1;
        }

        /* i16 checks */
        if(nw > UINT16_MAX)
        {   nw = UINT16_MAX;
        }
        *w = nw;
    }
    if(h)
    {
        int32_t nh = *h;

        if(nh <= 0)
        {   nh = 1;
        }
        /* i16 checks */
        if(nh > UINT16_MAX)
        {   nh = UINT16_MAX;
        }
        *h = nh;
    }
    return 0;
}

void
XCBWMHandlerKeyPress(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    (void)event;
}
void
XCBWMHandlerKeyRelease(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    (void)event;
}
void
XCBWMHandlerButtonPress(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBButtonPressEvent *ev         = (XCBButtonPressEvent *)event;
    const int16_t rootx             = ev->root_x;
    const int16_t rooty             = ev->root_y;
    const int16_t eventx            = ev->event_x;
    const int16_t eventy            = ev->event_y;
    const uint16_t state            = ev->state;
    const XCBWindow eventroot       = ev->root;
    const XCBWindow eventwin        = ev->event;
    const XCBWindow eventchild      = ev->child;
    const uint8_t samescreen        = ev->same_screen;
    const XCBKeyCode keydetail      = ev->detail;
    const XCBTimestamp tim          = ev->time;


    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)state;
    (void)eventchild;
    (void)samescreen;
    (void)keydetail;
    (void)tim;

    if(!display)
    {   return;
    }

    static XCBAtom netactive = 0;

    if(!netactive)
    {
        XCBCookie cookie = XCBInternAtomCookie(display, "_NET_ACTIVE_WINDOW", 0);
        netactive = XCBInternAtomReply(display, cookie);
    }

    /* unfocus any other windows */
    if(eventroot)
    {   
        XCBSetInputFocus(display, eventroot, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        if(netactive)
        {   XCBDeleteProperty(display, eventroot, netactive);
        }
    }
    /* focus window */
    if(eventwin)
    {   
        XCBSetInputFocus(display, eventwin, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        if(netactive && eventroot)
        {   XCBChangeProperty(display, eventroot, netactive, XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&(eventwin), 1);
        }
    }
    XCBFlush(display);
}
void
XCBWMHandlerButtonRelease(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBButtonPressEvent *ev = (XCBButtonPressEvent *)event;
    const int16_t rootx             = ev->root_x;
    const int16_t rooty             = ev->root_y;
    const int16_t eventx            = ev->event_x;
    const int16_t eventy            = ev->event_y;
    const uint16_t state            = ev->state;
    const XCBWindow eventroot       = ev->root;
    const XCBWindow eventwin        = ev->event;
    const XCBWindow eventchild      = ev->child;
    const uint8_t samescreen        = ev->same_screen;
    const XCBKeyCode keydetail      = ev->detail;
    const XCBTimestamp tim          = ev->time;


    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)state;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)samescreen;
    (void)keydetail;
    (void)tim;

    (void)ev;
}
void
XCBWMHandlerMotionNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBMotionNotifyEvent *ev = (XCBMotionNotifyEvent *)event;
    const int16_t rootx             = ev->root_x;
    const int16_t rooty             = ev->root_y;
    const int16_t eventx            = ev->event_x;
    const int16_t eventy            = ev->event_y;
    const uint16_t state            = ev->state;
    const XCBWindow eventroot       = ev->root;
    const XCBWindow eventwin        = ev->event;
    const XCBWindow eventchild      = ev->child;
    const uint8_t samescreen        = ev->same_screen;
    const XCBKeyCode keydetail      = ev->detail;
    const XCBTimestamp tim          = ev->time;

    (void)rootx;
    (void)rooty;
    (void)eventx;
    (void)eventy;
    (void)state;
    (void)eventroot;
    (void)eventwin;
    (void)eventchild;
    (void)samescreen;
    (void)keydetail;
    (void)tim;

    (void)ev;
}
void
XCBWMHandlerEnterNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
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

    (void)ev;
}
void
XCBWMHandlerLeaveNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
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

    (void)ev;
}
void
XCBWMHandlerFocusIn(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBFocusInEvent *ev         = (XCBFocusInEvent *)event;
    const uint8_t detail        = ev->detail;
    const XCBWindow eventwin    = ev->event;
    const uint8_t mode          = ev->mode;

    (void)detail;
    (void)eventwin;
    (void)mode;

    (void)ev;
}
void
XCBWMHandlerFocusOut(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBFocusOutEvent *ev        = (XCBFocusOutEvent *)event;
    const uint8_t detail        = ev->detail;
    const XCBWindow eventwin    = ev->event;
    const uint8_t mode          = ev->mode;

    (void)detail;
    (void)eventwin;
    (void)mode;

    (void)ev;
}
void
XCBWMHandlerKeymapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBKeymapNotifyEvent *ev    = (XCBKeymapNotifyEvent *)event;
    uint8_t *eventkeys          = ev->keys;        /* DONOT FREE */

    (void)eventkeys;
    (void)ev;
}
void
XCBWMHandlerExpose(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBExposeEvent *ev          = (XCBExposeEvent *)event;
    const XCBWindow win         = ev->window;
    const int16_t x             = ev->x;
    const int16_t y             = ev->y;
    const uint16_t w            = ev->width;
    const uint16_t h            = ev->height;
    const uint16_t count        = ev->count;

    (void)win;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)count;

    (void)ev;
}
void
XCBWMHandlerGraphicsExpose(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBGraphicsExposeEvent *ev = (XCBGraphicsExposeEvent *)event;
    const int16_t x                 = ev->x;
    const int16_t y                 = ev->y;
    const uint16_t w                = ev->width;
    const uint16_t h                = ev->height;
    const uint16_t count            = ev->count;
    const XCBDrawable drawable      = ev->drawable;
    const uint8_t majoropcode       = ev->major_opcode;
    const uint16_t minoropcode     = ev->minor_opcode;


    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)count;
    (void)drawable;
    (void)majoropcode;
    (void)minoropcode;

    (void)ev;
}
void
XCBWMHandlerNoExpose(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBExposeEvent *ev          = (XCBExposeEvent *)event;
    const int16_t x             = ev->x;
    const int16_t y             = ev->y;
    const uint16_t w            = ev->width;
    const uint16_t h            = ev->height;
    const uint16_t count        = ev->count;
    const XCBWindow win         = ev->window;

    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)count;
    (void)win;

    (void)ev;
}
void
XCBWMHandlerConfigureRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBConfigureRequestEvent *ev    = (XCBConfigureRequestEvent *)event;
    const int16_t x                 = ev->x;
    const int16_t y                 = ev->y;
    const uint16_t w                = ev->width;
    const uint16_t h                = ev->height;
    const uint16_t bw               = ev->border_width;
    const uint16_t mask             = ev->value_mask;
    const uint8_t stack             = ev->stack_mode;
    const XCBWindow win             = ev->window;
    const XCBWindow parent          = ev->parent;
    const XCBWindow sibling         = ev->sibling;

    (void)parent;

    if(!display)
    {   return;
    }

    XCBWindowChanges wc;

    wc.x = x;
    wc.y = y;
    wc.width = w;
    wc.height = h;
    wc.border_width = bw;
    wc.sibling = sibling;
    wc.stack_mode = stack;

    if(win)
    {   
        XCBConfigureWindow(display, win, mask, &wc);
        XCBFlush(display);
    }
}
void
XCBWMHandlerCirculateRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBCirculateRequestEvent *ev    = (XCBCirculateRequestEvent *)event;
    const XCBWindow win             = ev->window;
    const XCBWindow eventwin        = ev->event;
    const uint8_t place             = ev->place;

    (void)eventwin;

    if(!display)
    {   return;
    }

    switch(place)
    {   
        case XCB_CIRCULATE_RAISE_LOWEST:
            XCBCirculateSubwindows(display, win, XCB_CIRCULATE_RAISE_LOWEST);
            break;
        case XCB_CIRCULATE_LOWER_HIGHEST:
            XCBCirculateSubwindows(display, win, XCB_CIRCULATE_LOWER_HIGHEST);
            break;
        default:                                /* technically its a bitflip since its 1 | 0 but it will do. */
            XCBCirculateSubwindows(display, win, !!place);
            fprintf(stderr, "SubWindow Attempted is invalid attempting closest possible.\n");
            break;
    }
}
void
XCBWMHandlerMapRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBMapRequestEvent *ev  = (XCBMapRequestEvent *)event;
    const XCBWindow parent  = ev->parent;
    const XCBWindow win     = ev->window;

    (void)parent;

    if(!display)
    {   return;
    }

    if(win)
    {   
        XCBMapWindow(display, win);
        XCBFlush(display);
    }
}
void
XCBWMHandlerResizeRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBResizeRequestEvent *ev   = (XCBResizeRequestEvent *)event;
    const XCBWindow win         = ev->window;
    const uint16_t w            = ev->width;
    const uint16_t h            = ev->height;

    if(!display)
    {   return;
    }

    int32_t nw = w;    /* new width    */
    int32_t nh = h;    /* new height   */
    __boundcheck_dimentions(NULL, NULL, &nw, &nh);
    if(win)
    {   XCBResizeWindow(display, win, nw, nh);
    }
}
void
XCBWMHandlerCirculateNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBCirculateNotifyEvent *ev     = (XCBCirculateNotifyEvent *)event;
    const uint8_t place             = ev->place;
    const XCBWindow win             = ev->window;
    const XCBWindow eventwin        = ev->event;

    (void)place;
    (void)win;
    (void)eventwin;
    
    (void)ev;
}
void
XCBWMHandlerConfigureNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBConfigureNotifyEvent *ev     = (XCBConfigureNotifyEvent *)event;
    const XCBWindow eventwin        = ev->event;
    const XCBWindow win             = ev->window;
    const XCBWindow abovesibling    = ev->above_sibling;
    const int16_t x                 = ev->x;
    const int16_t y                 = ev->y;
    const uint16_t w                = ev->width;
    const uint16_t h                = ev->height;
    const uint16_t borderwidth      = ev->border_width;
    const uint8_t overrideredirect  = ev->override_redirect;

    (void)eventwin;
    (void)win;
    (void)abovesibling;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)borderwidth;
    (void)overrideredirect;

    (void)ev;
}
void
XCBWMHandlerCreateNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBCreateNotifyEvent *ev        = (XCBCreateNotifyEvent *)event;
    const uint8_t overrideredirect  = ev->override_redirect;
    const XCBWindow win             = ev->window;
    const XCBWindow parentwin       = ev->parent;
    const int16_t x                 = ev->x;
    const int16_t y                 = ev->y;
    const uint16_t w                = ev->width;
    const uint16_t h                = ev->height;
    const uint16_t bw               = ev->border_width;


    (void)overrideredirect;
    (void)win;
    (void)parentwin;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)bw;

    (void)ev;
}
void
XCBWMHandlerDestroyNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBDestroyNotifyEvent *ev = (XCBDestroyNotifyEvent *)event;
    const XCBWindow win         = ev->window;
    const XCBWindow eventwin    = ev->event;        /* The Event win is the window that sent the message */

    (void)eventwin;
    (void)win;

    (void)ev;
}
void
XCBWMHandlerMapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBMapNotifyEvent *ev = (XCBMapNotifyEvent *)event;
    const XCBWindow win             = ev->window;
    const XCBWindow eventwin        = ev->event;
    const uint8_t override_redirect = ev->override_redirect;

    (void)win;
    (void)eventwin;
    (void)override_redirect;

    (void)ev;
}
void
XCBWMHandlerGravityNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBGravityNotifyEvent *ev       = (XCBGravityNotifyEvent *)event;
    const XCBWindow eventwin        = ev->event;
    const XCBWindow win             = ev->window;
    const int16_t x                 = ev->x;
    const int16_t y                 = ev->y;

    (void)eventwin;
    (void)win;
    (void)x;
    (void)y;

    (void)ev;
}
void
XCBWMHandlerMappingNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBMappingNotifyEvent *ev = (XCBMappingNotifyEvent *)event;
    const XCBKeyCode first_keycode = ev->first_keycode;
    const uint8_t count            = ev->count;
    const uint8_t request          = ev->request;

    (void)count;
    (void)first_keycode;
    (void)request;

    (void)ev;
}
void
XCBWMHandlerUnmapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBUnmapNotifyEvent *ev = (XCBUnmapNotifyEvent *)event;
    const XCBWindow eventwin    = ev->event;
    const XCBWindow win         = ev->window;
    const uint8_t isconfigure   = ev->from_configure;

    (void)eventwin;
    (void)win;
    (void)isconfigure;

    (void)ev;
}
void
XCBWMHandlerVisibilityNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBVisibilityNotifyEvent *ev    = (XCBVisibilityNotifyEvent *)event;
    const XCBWindow win             = ev->window;
    const uint8_t state             = ev->state;

    (void)win;
    (void)state;

    (void)ev;
}
void
XCBWMHandlerReparentNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBReparentNotifyEvent *ev      = (XCBReparentNotifyEvent *)event;
    const XCBWindow parent          = ev->parent;
    const XCBWindow win             = ev->window;
    const XCBWindow eventwin        = ev->event;
    const int16_t x                 = ev->x;
    const int16_t y                 = ev->y;
    const uint8_t override_redirect = ev->override_redirect;

    (void)parent;
    (void)win;
    (void)eventwin;
    (void)x;
    (void)y;
    (void)override_redirect;

    (void)ev;
}
void
XCBWMHandlerColormapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBColormapNotifyEvent *ev      = (XCBColormapNotifyEvent *)event;
    const XCBWindow win             = ev->window;
    const uint8_t state             = ev->state;
    const XCBColormap colormap      = ev->colormap;
    const uint8_t new               = ev->_new;

    (void)win;
    (void)state;
    (void)colormap;
    (void)new;

    (void)ev;
}
void
XCBWMHandlerClientMessage(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBClientMessageEvent *ev       = (XCBClientMessageEvent *)event;
    const XCBWindow win             = ev->window;
    const XCBAtom atom              = ev->type;
    const uint8_t format            = ev->format;
    const XCBClientMessageData data = ev->data;     /* union "same" as xlib data8 -> b[20] data16 -> s[10] data32 = l[5] */

    if(!display || format != 32 || !atom)
    {   return;
    }

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

    /* net stuff */
    static XCBAtom netclosewindow = 0;
    static XCBAtom netmoveresizewindow = 0;
    XCBCookie cookie;

    if(!netclosewindow)
    {   
        cookie = XCBInternAtomCookie(display, "_NET_CLOSE_WINDOW", 0);
        netclosewindow = XCBInternAtomReply(display, cookie);
    }
    if(!netmoveresizewindow)
    {   
        cookie = XCBInternAtomCookie(display, "_NET_MOVERESIZE_WINDOW", 0);
        netmoveresizewindow = XCBInternAtomReply(display, cookie);
    }

    if(atom == netclosewindow)
    {   
        if(win)
        {   XCBKillClient(display, win);
        }
    }
    else if(atom == netmoveresizewindow)
    {
        const uint32_t gravity = data.data32[0];
        int32_t x = data.data32[1];
        int32_t y = data.data32[2];
        int32_t w = data.data32[3];
        int32_t h = data.data32[4];
        int32_t bw = data.data32[5];

        __apply_net_gravity(gravity, &x, &y, w, h, bw);
        __boundcheck_dimentions(&x, &y, &w, &h);
        __boundcheck_dimentions(NULL, NULL, &bw, NULL);
        if(win)
        {   
            XCBMoveResizeWindow(display, win, x, y, w, h);
            XCBSetWindowBorderWidth(display, win, bw);
        }
    }

}
void
XCBWMHandlerPropertyNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBPropertyNotifyEvent *ev = (XCBPropertyNotifyEvent *)event;
    const XCBAtom atom          = ev->atom;
    const XCBWindow win         = ev->window;
    const XCBTimestamp timestamp= ev->time;
    const uint16_t state             = ev->state;


    (void)atom;
    (void)win;
    (void)timestamp;
    (void)state;

    (void)ev;
}
void
XCBWMHandlerSelectionClear(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    XCBSelectionClearEvent *ev = (XCBSelectionClearEvent *)event;
    const XCBWindow owner       = ev->owner;
    const XCBAtom selection     = ev->selection;
    const XCBTimestamp tim      = ev->time;

    (void)owner;
    (void)selection;
    (void)tim;

    (void)ev;
}

void
XCBWMHandlerSelectionNotify(
        XCBDisplay *display,
        XCBGenericEvent *event
        )
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

    (void)ev;
}

void
XCBWMHandlerSelectionRequest(
        XCBDisplay *display,
        XCBGenericEvent *event
        )
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

    (void)ev;
}

void
XCBWMHandlerGenericEvent(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    (void)display;
}
void
XCBWMHandlerErrorHandler(
    XCBDisplay *display,
    XCBGenericEvent *event
    )
{
    (void)display;
    if(event)
    {   
        XCBGenericError *err = (XCBGenericError *)event;
        fprintf(stderr, "%s %s\n", XCBGetErrorMajorCodeText(err->major_code), XCBGetFullErrorText(err->error_code));
        fprintf(stderr, "error_code: [%d], major_code: [%d], minor_code: [%d]\n"
              "sequence: [%d], response_type: [%d], resource_id: [%d]\n"
              "full_sequence: [%d]\n"
              ,
           err->error_code, err->major_code, err->minor_code, 
           err->sequence, err->response_type, err->resource_id, 
           err->full_sequence);
    }
}
