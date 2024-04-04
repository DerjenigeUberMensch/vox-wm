#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <pthread.h>

#include "util.h"
#include "dwm.h"
#include "xcb_trl.h"
#include "config.h"
#include "toggle.h"



extern WM _wm;

void
UserStats(const Arg *arg)
{
}

void
FocusMonitor(const Arg *arg)
{
    Monitor *m;
    if(!_wm.mons)
    {   
        DEBUG0("There are no monitors, this should not be possible.");
        return;
    }
    if(!_wm.mons->next)
    {   DEBUG0("There is no other monitor to focus.");
    }

    if(!_wm.selmon)
    {   DEBUG0("No monitor selected in Context, this should not be possible");
    }

    if((m = dirtomon(arg->i)) == _wm.selmon)
    {   return;
    }

    if(_wm.selmon->desksel->sel)
    {   unfocus(_wm.selmon->desksel->sel, 0);
    }
    _wm.selmon = m;
    focus(NULL);
}

void
ChangeMasterWindow(const Arg *arg)
{
}

void
KillWindow(XCBDisplay *display, XCBWindow win)
{
    xcb_kill_client(display, win);
}

void
TerminateWindow(XCBDisplay *display, XCBWindow win)
{
    xcb_kill_client(display, win);
}

void
DragWindow(
    XCBDisplay *display, 
    XCBWindow win,
    const XCBKeyCode key_or_button)
{
    i16 ox, oy;     /*    old   */
    i16 x, y;       /*  current */
    i16 nx, ny;     /*    new   */

    XCBCookie gbpcookie = 
        XCBGrabPointerCookie(display, win, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
    XCBCookie qpcookie = XCBQueryPointerCookie(display, win);


    XCBGrabPointer *grb = XCBGrabPointerReply(display, gbpcookie);
    XCBQueryPointer *qp = XCBQueryPointerReply(display, qpcookie);


    if(!grb || grb->status != XCB_GRAB_STATUS_SUCCESS)
    {   
        if(grb)
        {   free(grb);
        }
        return;
    }
    if(!qp)
    {
        free(grb);
        return;
    }

    x = qp->root_x;
    y = qp->root_y;
    ox = qp->win_x;
    oy = qp->win_y;

    free(grb);
    free(qp);

    XCBMotionNotifyEvent *ev;
    u8 cleanev;
    u8 detail = 0;
    do
    {
        ev = (XCBMotionNotifyEvent *)XCBPollForEvent(display);
        cleanev = XCB_EVENT_RESPONSE_TYPE(ev);

        if(ev->event == win && cleanev == XCB_MOTION_NOTIFY)
        {
            nx = ox + (ev->root_x - x);
            ny = oy + (ev->root_y - y);
            XCBMoveWindow(display, win, nx, ny);
        }
        if(cleanev == XCB_BUTTON_RELEASE)
        {   
            detail = ev->detail;
        }
        free(ev);
    } while(cleanev != 0 && cleanev != XCB_BUTTON_RELEASE && detail != key_or_button);
}

void
Restart(const Arg *arg)
{
    restart();
    quit();
}

void
Restartf(const Arg *arg)
{
    restart();
}

void
Quit(const Arg *arg)
{
    quit();
}

void
_ResizeWindow(
    XCBDisplay *display, 
    XCBWindow win,
    const XCBKeyCode key_or_button
    ) /* resizemouse */
{
    i16 ox, oy;     /*    old   */
    i16 x, y;       /*  current */
    i16 nx, ny;     /*    new   */

    u16 ow, oh;     /*    old   */
    u16 nw, nh;     /*    new   */

    i8 horiz;       /* bounds checks    */
    i8 vert;        /* bounds checks    */

    const XCBCursor cur = XCB_NONE;
    XCBCookie qpcookie = XCBQueryPointerCookie(display, win);
    XCBCookie gpcookie = XCBGrabPointerCookie(display, win, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, cur, XCB_CURRENT_TIME);
    XCBCookie gmcookie = XCBGetGeometryCookie(display, win);

    XCBQueryPointer *qp = XCBQueryPointerReply(display, qpcookie);
    XCBGrabPointer *gb = XCBGrabPointerReply(display, gpcookie);
    XCBGeometry *gm = XCBGetGeometryReply(display, gmcookie);

    if(!qp)
    {   return;
    }
    if(!gb || gb->status != XCB_GRAB_STATUS_SUCCESS)
    {   free(qp);
        return;
    }
    if(!gm)
    {   
        free(qp); 
        free(gb);
        return;
    }

    horiz = qp->win_x < (gm->width / 2) ? -1 : 1;
    vert = qp->win_y < (gm->height / 2) ? -1 : 1;

    x = qp->win_x;
    y = qp->win_y; 

    ox = gm->x;
    oy = gm->y;

    ow = gm->width;
    oh = gm->height;


    XCBMotionNotifyEvent *ev;
    u8 cleanev = 0;
    u8 detail = 0;
    do
    {
        ev = (XCBMotionNotifyEvent *)XCBPollForEvent(display);
        if(!ev)
        {   continue;
        }
        cleanev = XCB_EVENT_RESPONSE_TYPE(ev);

        if(ev->event == win && cleanev == XCB_MOTION_NOTIFY)
        {
            nw = ow + (horiz    * (ev->root_x - x));
            nh = oh + (vert     * (ev->root_y - y));
            nx = ox + (!~horiz) * (ow - nw);
            ny = oy + (!~vert)  * (oh - nh);
            XCBMoveResizeWindow(display, win, nx, ny, nw, nh);
        }
        detail = ev->detail;
        free(ev);
        ev = NULL;
    } while(cleanev != 0 && (cleanev != XCB_BUTTON_RELEASE && detail != key_or_button));
}

void
ResizeWindow(const Arg *arg)
{
    if(_wm.selmon->desksel->sel)
    {
        XCBButtonPressEvent *ev = arg->v;
        _ResizeWindow(_wm.dpy, _wm.selmon->desksel->sel->win, ev->detail);
    }
}

void
SetWindowLayout(const Arg *arg)
{
    const Monitor *m = _wm.selmon;

    if(!m) return;
    setdesktoplayout(m->desksel, arg->i);
    arrange(m->desksel);
}

void
SetMonitorFact(const Arg *arg)
{
}

void
SpawnWindow(const Arg *arg)
{
    struct sigaction sa;
    if (fork() == 0)
    {
        if (_wm.dpy)
            close(XCBConnectionNumber(_wm.dpy));
        setsid();
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;
        sigaction(SIGCHLD, &sa, NULL);
        execvp(((char **)arg->v)[0], (char **)arg->v);
        /* UNREACHABLE */
        DEBUG("%s", "execvp Failed");
    }
}

void
MaximizeWindow(const Arg *arg)
{
}

void
MaximizeWindowVertical(const Arg *arg) 
{
}

void
MaximizeWindowHorizontal(const Arg *arg) 
{
}

void
AltTab(const Arg *arg)
{
}

void
ToggleStatusBar(const Arg *arg)
{
    setshowbar(_wm.selmon, !SHOWBAR(_wm.selmon));
    updatebarpos(_wm.selmon);
    XCBMoveResizeWindow(_wm.dpy, _wm.selmon->barwin, _wm.selmon->wx, _wm.selmon->by, _wm.selmon->ww, _wm.selmon->bh);
    arrange(_wm.selmon->desksel);
}

void
ToggleFullscreen(const Arg *arg)
{
}


