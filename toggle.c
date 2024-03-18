#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <pthread.h>

#include "util.h"
#include "dwm.h"
#include "xcb_trl.h"
#include "config.h"
#include "toggle.h"



extern WM *_wm;

void
UserStats(const Arg *arg)
{
}

void
FocusMonitor(const Arg *arg)
{
}

void
ChangeMasterWindow(const Arg *arg)
{
}

void
KillWindow(const Arg *arg)
{
}

void
TerminateWindow(const Arg *arg)
{
}

void
DragWindow(XCBDisplay *display, XCBWindow win, const XCBKeyCode key_or_button)
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

        if(cleanev == XCB_MOTION_NOTIFY)
        {
            nx = ox + (ev->root_x - x);
            ny = oy + (ev->root_y - y);
            XCBMoveWindow(display, win, nx, ny);
        }
        if(cleanev == XCB_BUTTON_RELEASE)
        {   
            detail = (XCBButtonReleaseEvent *)ev->detail;
        }
        free(ev);
    } while(cleanev != 0 && cleanev != XCB_BUTTON_RELEASE && detail != key_or_button);
}

void
Restart(const Arg *arg)
{
    restart();
    /* this just generates a event to wakeup the thread */
    XCBMapWindow(_wm->dpy, _wm->root);
}

void
Quit(const Arg *arg)
{   
    quit();
    /* this just generates a event to wakeup the thread */
    XCBMapWindow(_wm->dpy, _wm->root);
}

void
ResizeWindow(const Arg *arg) /* resizemouse */
{
    i16 ox, oy;     /*    old   */
    i16 x, y;       /*  current */
    i16 nx, ny;     /*    new   */

    u16 ow, oh;     /*    old   */
    u16 w, h;       /*  current */
    u16 nw, nh;     /*    new   */
}

void
SetWindowLayout(const Arg *arg)
{
}

void
SetMonitorFact(const Arg *arg)
{
}

void
SpawnWindow(const Arg *arg)
{
    if (fork() == 0)
    {
        if (_wm->dpy)
            close(XCBConnectionNumber(_wm->dpy));
        setsid();
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
    setshowbar(_wm->selmon, !SHOWBAR(_wm->selmon));
    updatebarpos(_wm->selmon);
    XCBMoveResizeWindow(_wm->dpy, _wm->selmon->barwin, _wm->selmon->wx, _wm->selmon->by, _wm->selmon->ww, _wm->selmon->bh);
    arrange(_wm->selmon->desksel);
}

void
ToggleFullscreen(const Arg *arg)
{
}
