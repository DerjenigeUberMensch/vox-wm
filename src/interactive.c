#include <X11/cursorfont.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xproto.h>

#include "XCB-TRL/xcb_trl.h"
#include "XCB-TRL/xcb_trl_types.h"
#include "client.h"
#include "cursor.h"
#include "main.h"
#include "floating.h"

extern WM _wm;
extern UserSettings _cfg;
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];
extern XCBCursor x11_cursors[XC_num_glyphs];
extern XCBCursor x11_modern_cursors[XC_num_glyphs];

STATIC_ASSERT(XCBNone == 0, XCB_NONE_MUST_BE_ZERO);

struct
InteractiveDrag
{
    XCBWindow window;
    XCBButton detail;

    i16 nx, ny;
    i16 x, y;

    i16 oldx, oldy;
    u16 oldw, oldh;

    u16 bw;

    XCBTimestamp lasttime;
    XCBCursor cursor;
};

struct
InteractiveResize
{
    XCBWindow window;
    XCBButton detail;

    i16 nx, ny;
    i16 x, y;

    i32 nw, nh;

    i16 oldx, oldy;
    u16 oldw, oldh;

    i16 curx, cury;

    i8 horz, vert;

    u16 minw, minh;
    u16 maxw, maxh;

    u16 bw;

    u8 altmode;

    XCBTimestamp lasttime;
    XCBCursor cursor;
};

static int DragWindowHandler(XCBGenericEvent *event, Arg arg);
static int ResizeWindowHandler(XCBGenericEvent *event, Arg arg);

int
DragWindow(XCBWindow window, XCBButton button)
{
    if(window == XCBNone || IS_WM_WINDOW(window))
    {   
        DebugWarn("Invalid window passed to DragWindow");
        return EXIT_FAILURE;
    }

    Debug("Called.");

    int status;
    Arg data = {0};
    struct InteractiveDrag *drag;

    data.v = malloc(sizeof(struct InteractiveDrag));

    if(!data.v)
    {   
        DebugWarn("Failed to start %s", __func__);
        return EXIT_FAILURE;
    }

    /*
     * Work is already registered with the event queue.
     * Mark it invalid so DragWindowHandler terminates and lets
     * the work queue reclaim its argument on the next dispatch.
     *
     * - chatgpt
     * (chatgpt saying to add comemnt ehre for no god damnn reason)
     */
    memset(data.v, 0, sizeof(struct InteractiveDrag));

    status = WM_ADD_WORK(DragWindowHandler, data, true);

    if(status == EXIT_FAILURE)
    {
        DebugWarn("Failed to add work for DragWindowHandler");
        free(data.v);

        return EXIT_FAILURE;
    }

    drag = data.v;

    drag->window = window;
    drag->detail = CLEANBUTTONMASK(button);
    drag->cursor = XCBNone;

#ifdef DEBUG
    if(!wintoclient(window))
    {   DebugLog("Window has no associated client, window may not exist");
    }
#endif

    /* some themes seem to just use the default pointer for some reason??????*/
    /* TryGetCursor(WM_cursor_move); */
    XCBCursor cursor = TryGetCursor(XC_fleur);

    drag->cursor = cursor;   

    XCBCookie GrabPointerCookie = XCBGrabPointerCookie(_wm.dpy, _wm.root, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCBNone, drag->cursor, XCB_CURRENT_TIME);
    XCBCookie QueryPointerCookie = XCBQueryPointerCookie(_wm.dpy, window);
    /* redudant in 1 path but basicaly a free query anyways */
    XCBCookie GetGeometryCookie = XCBGetGeometryCookie(_wm.dpy, window);

    XCBGrabPointer *GrabPointer = XCBGrabPointerReply(_wm.dpy, GrabPointerCookie);

    /* FIXME this looks horrible */
    if(!GrabPointer || GrabPointer->status != XCB_GRAB_STATUS_SUCCESS)
    {   
        free(GrabPointer);

        XCBDiscardReply(_wm.dpy, QueryPointerCookie);
        XCBDiscardReply(_wm.dpy, GetGeometryCookie);

        goto DESTROY;
    }

    free(GrabPointer);

    XCBQueryPointer *pointer = XCBQueryPointerReply(_wm.dpy, QueryPointerCookie);

    if(!pointer)
    {   
        XCBDiscardReply(_wm.dpy, GetGeometryCookie);
        XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);

        goto DESTROY;
    }

    drag->x = pointer->root_x;
    drag->y = pointer->root_y;

    free(pointer);

    Client *c = wintoclient(window);

    if(c)
    {
        drag->oldx = c->x;
        drag->oldy = c->y;
        drag->oldw = c->w;
        drag->oldh = c->h;
        drag->bw = c->bw;

        XCBDiscardReply(_wm.dpy, GetGeometryCookie);
    }
    else
    {

        XCBGeometry *geom = XCBGetGeometryReply(_wm.dpy, GetGeometryCookie);

        if(!geom)   
        {   
            XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);
            goto DESTROY;
        }

        drag->oldx = geom->x;
        drag->oldy = geom->y;
        drag->oldw = geom->width;
        drag->oldh = geom->height;
        drag->bw = geom->border_width;

        free(geom);
    }

    /* make sure calculations include border width */
    drag->oldw += drag->bw * 2;
    drag->oldh += drag->bw * 2;

    if(c)
    {
        /* prevent DOCKED from computing non floating */

        setfloating(c, 1); 
        /* make sure auto-docking dosent auto dock it */
        c->x += 1;

        arrange(c->desktop);

        /* soemtimes clients are below other sif they spwaned in later */
        /* TODO: This is a quick fix, and prob should just reowkr restack but wahtrever */
        if(moncount() > 1)
        {   restackc(c->desktop, 1);
        }
    }
    /* non managed clients cant possible always be in view so raise it */
    else
    {   XCBRaiseWindow(_wm.dpy, window);
    }

    XCBFlush(_wm.dpy);

    return EXIT_SUCCESS;
DESTROY:
    /* drag will be set to nothign and thus our handler should not attempt to use it */
    if(drag)
    {   memset(drag, 0, sizeof(struct InteractiveDrag));
    }

    XCBFlush(_wm.dpy);

    return EXIT_FAILURE;
}

static int 
DragWindowHandler(XCBGenericEvent *event, Arg arg)
{
    bool running = true;

    if(unlikely(!arg.v))
    {   
        running = false;
        return running;
    }

    struct InteractiveDrag *drag = arg.v;
    Client *c;
    XCBMotionNotifyEvent *mev = NULL;
    XCBButtonReleaseEvent *brev = NULL;
    XCBUnmapNotifyEvent *umev = NULL;
    XCBDestroyNotifyEvent *dnev = NULL;
    u16 refreshrate = 0;
    u16 snap = 0;
    Monitor *m;
    u32 moncount = 0;

    if(drag->window == XCBNone)
    {   
        running = false;
        return running;
    }

    enum
    {
        _NET_WM_MOVERESIZE_SIZE_TOPLEFT,
        _NET_WM_MOVERESIZE_SIZE_TOP,
        _NET_WM_MOVERESIZE_SIZE_TOPRIGHT,
        _NET_WM_MOVERESIZE_SIZE_RIGHT,
        _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT,
        _NET_WM_MOVERESIZE_SIZE_BOTTOM,
        _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT,
        _NET_WM_MOVERESIZE_SIZE_LEFT,
        _NET_WM_MOVERESIZE_MOVE,            /* movement only */
        _NET_WM_MOVERESIZE_SIZE_KEYBOARD,   /* size via keyboard */
        _NET_WM_MOVERESIZE_MOVE_KEYBOARD,   /* move via keyboard */
        _NET_WM_MOVERESIZE_CANCEL,          /* cancel operation */
    };

    switch(XCB_EVENT_RESPONSE_TYPE(event))
    {
        case XCB_MOTION_NOTIFY:
            mev = (XCBMotionNotifyEvent *)event;
            refreshrate = USGetSetting(&_cfg, RefreshRate).data16[0];
            snap = USGetSetting(&_cfg, Snap).data16[0];

            if(refreshrate)
            {
                /* X time is in milieeconds so devide by 1000 to get frame time in seconds */
                const float FRAME_TIME = 1000.0f / refreshrate;

                if((mev->time - drag->lasttime) <= FRAME_TIME)
                {   break;
                }

                drag->lasttime = mev->time;
            }

            drag->nx = drag->oldx + mev->event_x - drag->x;
            drag->ny = drag->oldy + mev->event_y - drag->y;

            c = wintoclient(drag->window);

            if(c && (unlikely(drag->oldw != c->w || drag->oldh != c->h)))
            {   
                drag->oldw = c->w;
                drag->oldh = c->h;

                /* winit... alacrity... rust... */
                DebugWarn("\"%s\" [%d] changed its sized while dragging ",
                        c->netwmname ? c->netwmname : c->wmname ? c->wmname : "UNKNOWN",
                        c->win
                        );
            }

            m = recttomon(drag->nx, drag->ny, drag->oldw, drag->oldh);

            m = m ? m : _wm.selmon;

            enum { SNAP_DOESNT_BREAK_ON_ONE_MONITOR = 1 };
            moncount = rectmoncount(drag->nx, drag->ny, drag->oldw, drag->oldh);

            /* do nothing, TODO: Make this look pretty */
            if(moncount > SNAP_DOESNT_BREAK_ON_ONE_MONITOR)
            {   (void)0;
            }
            /* snap to window area */
            else if (abs(m->wx - drag->nx) < snap)
            {   drag->nx = m->wx;
            }
            else if (abs((m->wx + m->ww) - (drag->nx + drag->oldw)) < snap)
            {   drag->nx = m->wx + m->ww - drag->oldw;
            }
            if (abs(m->wy - drag->ny) < snap)
            {   drag->ny = m->wy;
            }
            else if (abs((m->wy + m->wh) - (drag->ny + drag->oldh)) < snap)
            {   drag->ny = m->wy + m->wh - drag->oldh;
            }

            if(c)
            {   resizemove(c, drag->nx, drag->ny, 1);
            }
            else
            {   XCBMoveWindow(_wm.dpy, drag->window, drag->nx, drag->ny);
            }

            XCBFlush(_wm.dpy);
            break;
        case XCB_CLIENT_MESSAGE:
        {
            XCBClientMessageEvent *ev = (XCBClientMessageEvent *)event;
            const XCBWindow win = ev->window;
            const XCBAtom atom = ev->type;
            const u8 format = ev->format;
            /* union "same" as xlib data8 -> b[20] data16 -> s[10] data32 = l[5] */
            const XCBClientMessageData data = ev->data; 

            if(format != 32 || win != drag->window || atom != netatom[NetMoveResize])
            {   break;
            }

            const u32 l0 = data.data32[0];
            const u32 l1 = data.data32[1];
            const u32 l2 = data.data32[2];
            const u32 l3 = data.data32[3];
            const u32 l4 = data.data32[4];

            (void)l0;
            (void)l1;
            (void)l3;
            (void)l4;

            const int netwmstate = l2;

            if(netwmstate == _NET_WM_MOVERESIZE_CANCEL)
            {   running = false;
            }

            break;
        }
        case XCB_BUTTON_RELEASE:
            brev = (XCBButtonReleaseEvent *)event;

            if(CLEANBUTTONMASK(brev->detail) == drag->detail || drag->detail == XCBButtonAny)
            {   running = false;
            }

            break;
            /* this accounts for users killing the window (cause they can) */
        case XCB_UNMAP_NOTIFY:
            umev = (XCBUnmapNotifyEvent *)event;

            if(umev->window == drag->window)
            {   running = false;
            }

            break;
        case XCB_DESTROY_NOTIFY:
            dnev = (XCBDestroyNotifyEvent *)event;

            if(dnev->window == drag->window)
            {   running = false;
            }

            break;
    }

    if(!running)
    {
        XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);

        c = wintoclient(drag->window);

        m = NULL;

        if(c)
        {
            m = recttomon(c->x, c->y, c->w, c->h);

            if(m && m != c->desktop->mon)
            {   setclientdesktop(c, m->desksel);
            }

            if(DOCKED(c))
            {   setfloating(c, 0);
            }

            focus(c);
        }

        arrange(m ? m->desksel : _wm.selmon->desksel);
        XCBFlush(_wm.dpy);
    }

    return running;
}

int
ResizeWindowHandler(
        XCBGenericEvent *event,
        Arg arg
        )
{
    bool running = true;

    if(unlikely(!arg.v))
    {   
        running = false;
        return running;
    }

    struct InteractiveResize *rsz = arg.v;

    if(rsz->window == XCBNone)
    {   
        running = false;
        return running;
    }

    enum
    {
        _NET_WM_MOVERESIZE_SIZE_TOPLEFT,
        _NET_WM_MOVERESIZE_SIZE_TOP,
        _NET_WM_MOVERESIZE_SIZE_TOPRIGHT,
        _NET_WM_MOVERESIZE_SIZE_RIGHT,
        _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT,
        _NET_WM_MOVERESIZE_SIZE_BOTTOM,
        _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT,
        _NET_WM_MOVERESIZE_SIZE_LEFT,
        _NET_WM_MOVERESIZE_MOVE,            /* movement only */
        _NET_WM_MOVERESIZE_SIZE_KEYBOARD,   /* size via keyboard */
        _NET_WM_MOVERESIZE_MOVE_KEYBOARD,   /* move via keyboard */
        _NET_WM_MOVERESIZE_CANCEL,          /* cancel operation */
    };

    Client *c;
    XCBMotionNotifyEvent *mev = NULL;
    XCBButtonReleaseEvent *brev = NULL;
    XCBUnmapNotifyEvent *umev = NULL;
    XCBDestroyNotifyEvent *dnev = NULL;
    u16 refreshrate = 0;

    switch(XCB_EVENT_RESPONSE_TYPE(event))
    {   
        case XCB_MOTION_NOTIFY:
            mev = (XCBMotionNotifyEvent *)event;
            refreshrate = USGetSetting(&_cfg, RefreshRate).data16[0];

            if(refreshrate)
            {
                const float FRAME_TIME = 1000.0f / refreshrate;

                if((mev->time - rsz->lasttime) <= FRAME_TIME)
                {   break;
                }
                
                rsz->lasttime = mev->time;
            }

            rsz->nw = rsz->oldw + rsz->horz * (mev->root_x - rsz->curx);
            rsz->nh = rsz->oldh + rsz->vert * (mev->root_y - rsz->cury);

            if(!rsz->altmode)
            {
                if(rsz->maxw)
                {   rsz->nw = MIN(rsz->nw, rsz->maxw);
                }

                if(rsz->maxh)
                {   rsz->nh = MIN(rsz->nh, rsz->maxh);
                }

                rsz->nw = MAX(rsz->nw, rsz->minw);
                rsz->nh = MAX(rsz->nh, rsz->minh);
            }

            /* prevent glitching */
            rsz->nw = MAX(rsz->nw, 0);
            rsz->nh = MAX(rsz->nh, 0);

            rsz->nx = rsz->oldx + !~rsz->horz * (rsz->oldw - rsz->nw);
            rsz->ny = rsz->oldy + !~rsz->vert * (rsz->oldh - rsz->nh);

            c = wintoclient(rsz->window);

            if(c)
            {   
                if(!rsz->altmode)
                {   resize(c, rsz->nx, rsz->ny, rsz->nw, rsz->nh, 1);
                }
                else
                {   resizeclient(c, rsz->nx, rsz->ny, rsz->nw, rsz->nh);
                }
            }
            else
            {   XCBMoveResizeWindow(_wm.dpy, rsz->window, rsz->nx, rsz->ny, rsz->nw, rsz->nh);
            }

            XCBFlush(_wm.dpy);
            break;
        case XCB_CLIENT_MESSAGE:
        {
            XCBClientMessageEvent *ev = (XCBClientMessageEvent *)event;
            const XCBWindow win = ev->window;
            const XCBAtom atom = ev->type;
            const u8 format = ev->format;
            /* union "same" as xlib data8 -> b[20] data16 -> s[10] data32 = l[5] */
            const XCBClientMessageData data = ev->data; 

            if(format != 32 || win != rsz->window || atom != netatom[NetMoveResize])
            {   break;
            }

            const u32 l0 = data.data32[0];
            const u32 l1 = data.data32[1];
            const u32 l2 = data.data32[2];
            const u32 l3 = data.data32[3];
            const u32 l4 = data.data32[4];

            (void)l0;
            (void)l1;
            (void)l3;
            (void)l4;

            const int netwmstate = l2;

            if(netwmstate == _NET_WM_MOVERESIZE_CANCEL)
            {   running = false;
            }

            break;
        }
        case XCB_BUTTON_RELEASE:
            brev = (XCBButtonReleaseEvent *)event;

            if(CLEANBUTTONMASK(brev->detail) == rsz->detail || rsz->detail == XCBButtonAny)
            {   running = false;
            }

            break;
            /* this accounts for users killing the window (cause they can) */
        case XCB_UNMAP_NOTIFY:
            umev = (XCBUnmapNotifyEvent *)event;

            if(umev->window == rsz->window)
            {   running = false;
            }

            break;
        case XCB_DESTROY_NOTIFY:
            dnev = (XCBDestroyNotifyEvent *)event;

            if(dnev->window == rsz->window)
            {   running = false;
            }

            break;
    }

    if(!running)
    {
        XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);

        Monitor *m = NULL;

        c = wintoclient(rsz->window);

        if(c)
        {
            m = recttomon(c->x, c->y, c->w, c->h);

            if(m && m != c->desktop->mon)
            {   setclientdesktop(c, m->desksel);
            }

            if(DOCKED(c))
            {   setfloating(c, 0);
            }

            focus(c);
        }

        arrange(m ? m->desksel : _wm.selmon->desksel);
        XCBFlush(_wm.dpy);
    }

    return running;
}

int
ResizeWindow(XCBWindow window, XCBButton button, bool alt_mode)
{
    if(window == XCBNone || IS_WM_WINDOW(window))
    {   
        DebugWarn("Invalid window passed to %s", __func__);
        return EXIT_FAILURE;
    }

    Debug("Called.");

    int status;
    Arg data = {0};
    struct InteractiveResize *rsz;

    data.v = malloc(sizeof(struct InteractiveResize));

    if(!data.v)
    {   
        DebugWarn("Failed to start %s", __func__);
        return EXIT_FAILURE;
    }

    /*
     * Work is already registered with the event queue.
     * Mark it invalid so DragWindowHandler terminates and lets
     * the work queue reclaim its argument on the next dispatch.
     *
     * - chatgpt
     * (chatgpt saying to add comemnt ehre for no god damnn reason)
     */
    memset(data.v, 0, sizeof(struct InteractiveResize));

    status = WM_ADD_WORK(ResizeWindowHandler, data, true);

    if(status == EXIT_FAILURE)
    {
        DebugWarn("Failed to add work for %s", __func__);
        free(data.v);

        return EXIT_FAILURE;
    }

    rsz = data.v;

    rsz->window = window;
    rsz->detail = CLEANBUTTONMASK(button);
    rsz->cursor = XCBNone;
    rsz->altmode = alt_mode;

#ifdef DEBUG
    if(!wintoclient(window))
    {   DebugLog("Window has no associated client, window may not exist");
    }
#endif

    XCBCookie QueryPointerCookie = XCBQueryPointerCookie(_wm.dpy, window);
    /* redudant in 1 path but basicaly a free query anyways */
    XCBCookie GetGeometryCookie = XCBGetGeometryCookie(_wm.dpy, window);
    XCBCookie GetWMNormalHintsCookie = XCBGetWMNormalHintsCookie(_wm.dpy, window);

    /* --------------------------------------------- */

    XCBQueryPointer *pointer = XCBQueryPointerReply(_wm.dpy, QueryPointerCookie);

    if(!pointer)
    {   
        XCBDiscardReply(_wm.dpy, GetGeometryCookie);
        XCBDiscardReply(_wm.dpy, GetWMNormalHintsCookie);

        goto DESTROY;
    }

    rsz->curx = pointer->root_x;
    rsz->cury = pointer->root_y;
    rsz->nx = pointer->win_x;
    rsz->ny = pointer->win_y;

    free(pointer);

    Client *c = wintoclient(window);

    if(c)
    {
        rsz->oldx = c->x;
        rsz->oldy = c->y;
        rsz->oldw = c->w;
        rsz->oldh = c->h;
        rsz->bw = c->bw;
        rsz->minw = c->minw;
        rsz->minh = c->minh;
        rsz->maxw = c->maxw;
        rsz->maxh = c->maxh;

        XCBDiscardReply(_wm.dpy, GetGeometryCookie);
        XCBDiscardReply(_wm.dpy, GetWMNormalHintsCookie);
    }
    else
    {
        XCBGeometry *geom = XCBGetGeometryReply(_wm.dpy, GetGeometryCookie);

        if(!geom)   
        {   
            XCBDiscardReply(_wm.dpy, GetWMNormalHintsCookie);
            goto DESTROY;
        }

        rsz->oldx = geom->x;
        rsz->oldy = geom->y;
        rsz->oldw = geom->width;
        rsz->oldh = geom->height;
        rsz->bw = geom->border_width;

        free(geom);

        XCBSizeHints hints;
        u8 hintsstatus = XCBGetWMNormalHintsReply(_wm.dpy, GetWMNormalHintsCookie, &hints);

        Client c1 = {0};

        if(hintsstatus)
        {   updatesizehints(&c1, &hints);
        }
        else
        {
            const int MIN_SIZE = 1;

            c1.minw = MIN_SIZE;
            c1.minh = MIN_SIZE;

            c1.maxw = 0;
            c1.maxh = 0;
        }

        rsz->minw = c1.minw;
        rsz->minh = c1.minh;
        rsz->maxw = c1.maxw;
        rsz->maxh = c1.maxh;
    }

    const u8 MIN_SIZE = 5;

    rsz->minw = MAX(rsz->minw, MIN_SIZE);
    rsz->minh = MAX(rsz->minh, MIN_SIZE);

    rsz->horz = rsz->nx < rsz->oldw / 2 ? -1 : 1;
    rsz->vert = rsz->ny < rsz->oldh / 2 ? -1 : 1;

    XCBCursor cursor = XCBNone;

    if(rsz->horz == -1)
    {
        /* top left */
        if(rsz->vert == -1)
        {   cursor = TryGetCursor(WM_cursor_resize_diagonal_left);
        }
        /* Bottom Right */
        else
        {   cursor = TryGetCursor(WM_cursor_resize_diagonal_right);
        }
    }
    else
    {
        /* top right */
        if(rsz->vert == -1)
        {   cursor = TryGetCursor(WM_cursor_resize_diagonal_right);
        }
        /* bottom left */
        else
        {   cursor = TryGetCursor(WM_cursor_resize_diagonal_left);
        }
    }

    rsz->cursor = cursor;

    XCBCookie GrabPointerCookie = XCBGrabPointerCookie(_wm.dpy, _wm.root, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCBNone, rsz->cursor, XCB_CURRENT_TIME);

    XCBGrabPointer *GrabPointer = XCBGrabPointerReply(_wm.dpy, GrabPointerCookie);


    if(!GrabPointer || GrabPointer->status != XCB_GRAB_STATUS_SUCCESS)
    {   goto DESTROY;
    }

    if(c)
    {
        /* prevent DOCKED from computing non floating */
        setfloating(c, 1); 
        /* make sure auto-docking dosent auto dock it */
        c->x += 1;

        arrange(c->desktop);

        /* soemtimes clients are below other sif they spwaned in later */
        /* TODO: This is a quick fix, and prob should just reowkr restack but wahtrever */
        if(moncount() > 1)
        {   restackc(c->desktop, 1);
        }
    }
    /* non managed clients cant possible always be in view so raise it */
    else
    {   XCBRaiseWindow(_wm.dpy, window);
    }

    XCBFlush(_wm.dpy);

    return EXIT_SUCCESS;
DESTROY:
    /* resize will be set to nothign and thus our handler should not attempt to use it */
    if(rsz)
    {   memset(rsz, 0, sizeof(struct InteractiveResize));
    }

    XCBFlush(_wm.dpy);

    return EXIT_FAILURE;
}