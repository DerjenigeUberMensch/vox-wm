#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

#include <pthread.h>

#include "util.h"
#include "dwm.h"
#include "toggle.h"

/*
 * For people wanting to make new functions:
 * XCB buffers requests to the display (for some of them not all (which is dumb btw.)) so you just do a bunch of stuff then when your done just do XCBFlush();
 */

/* TODO: Make these functions seperate threads */
extern void (*handler[XCBLASTEvent]) (XCBGenericEvent *);
extern WM _wm;
extern XCBCursor cursors[CurLast];

void
UserStats(const Arg *arg)
{
    Client *c = _wm.selmon->desksel->sel;

    if(c)
    {   DEBUG("(x: %d, y: %d, w: %u, h: %u)", c->x, c->y, c->w, c->h);
        DEBUG("(ox: %d, oy: %d, ow: %u, oh: %u)", c->oldx, c->oldy, c->oldw, c->oldh);
        DEBUG("NETNAME:     %s", c->netwmname);
        DEBUG("WMNAME:      %s", c->wmname);
        DEBUG("CLASSNAME:   %s", c->classname);
        DEBUG("INSTANCENAME:%s", c->instancename);
    }
    else
    {   DEBUG0("NULL");
    }
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
    XCBFlush(_wm.dpy);
}

void
ChangeMasterWindow(const Arg *arg)
{
}

void
KillWindow(const Arg *arg)
{
    if(_wm.selmon->desksel->sel)
    {
        killclient(_wm.selmon->desksel->sel, Graceful);
        XCBFlush(_wm.dpy);
    }
}

void
TerminateWindow(const Arg *arg)
{
    if(_wm.selmon->desksel->sel)
    {
        killclient(_wm.selmon->desksel->sel, Destroy);
        XCBFlush(_wm.dpy);
    }
}

void
DragWindow(
        const Arg *arg
        )
{

    if(!_wm.selmon->desksel->sel)
    {   return;
    }
    Client *c = _wm.selmon->desksel->sel;
    XCBWindow win = c->win;
    i16 nx, ny;
    i16 x, y;
    i16 oldx, oldy;
    const XCBCursor cur = XCB_NONE;

    /* init data */
    x = y = 0;
    oldx = c->x;
    oldy = c->y;

    XCBCookie QueryPointerCookie = XCBQueryPointerCookie(_wm.dpy, win);
    XCBQueryPointer *pointer = XCBQueryPointerReply(_wm.dpy, QueryPointerCookie);

    if(pointer)
    {
        x = pointer->root_x;
        y = pointer->root_y;
        free(pointer);
    }
    XCBCookie GrabPointerCookie = XCBGrabPointerCookie(_wm.dpy, _wm.root, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, None, cur, XCB_CURRENT_TIME);
    XCBGrabPointer *GrabPointer = XCBGrabPointerReply(_wm.dpy, GrabPointerCookie);

    if(!GrabPointer || GrabPointer->status != XCB_GRAB_STATUS_SUCCESS)
    {   free(GrabPointer);
        return;
    }
    /* prevent DOCKED from computing non floating */
    setfloating(c, 1); c->x += 1;
    arrange(c->desktop);
    XCBRaiseWindow(_wm.dpy, win);
    XCBFlush(_wm.dpy);
    XCBGenericEvent *ev = NULL;
    do
    {
        if(ev)
        {
            eventhandler(ev);
            if(XCB_EVENT_RESPONSE_TYPE(ev) == XCB_MOTION_NOTIFY)
            {
                XCBMotionNotifyEvent *mev = (XCBMotionNotifyEvent *)ev;
                nx = oldx + (mev->event_x - x);
                ny = oldy + (mev->event_y - y);
                resize(c, nx, ny, c->w, c->h, 1);
                XCBFlush(_wm.dpy);
                DEBUG("(x: %d, y: %d)", nx, ny);
            }
            /* this accounts for users killing the window (cause they can) */
            else if(XCB_EVENT_RESPONSE_TYPE(ev) == XCB_UNMAP_NOTIFY)
            {   XCBUnMapNotifyEvent *uev = (XCBUnMapNotifyEvent *)ev;
                if(uev->window == win)
                {   free(ev);
                    win = 0;
                    break;
                }
            }
            free(ev);
        }
    } while(_wm.running && !XCBNextEvent(_wm.dpy, &ev) && XCB_EVENT_RESPONSE_TYPE(ev) != XCB_BUTTON_RELEASE);
    XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);
    Monitor *m;
    if(win)
    {
        if ((m = recttomon(c->x, c->y, c->w, c->h)) != _wm.selmon) 
        {
            setclientdesktop(c, m->desksel);
            _wm.selmon = m;
            focus(NULL);
        }
        if(DOCKED(c))
        {   setfloating(c, 0);
        }
    }
    arrange(_wm.selmon->desksel);
    XCBFlush(_wm.dpy);
}

void
Restart(const Arg *arg)
{
    restart();
    quit();
}

void
Quit(const Arg *arg)
{
    quit();
}

void
ResizeWindow(const Arg *arg)
{
    if(!_wm.selmon->desksel->sel)
    {   return;
    }
    Client *c = _wm.selmon->desksel->sel;

    XCBDisplay *display = _wm.dpy;
    XCBWindow win = c->win;
    const int MIN_SIZE = 1;

    i16 curx, cury;
    i32 oldw, oldh;
    i16 nx, ny;
    i32 nw, nh;
    i16 oldx, oldy;
    i8 horz, vert;
    u16 basew, baseh;
    XCBCursor cur;

    /* init data */
    curx = cury = oldw = oldh = nx = ny = nw = nh = oldx = oldy = horz = vert = basew = baseh = 0;

    XCBCookie QueryPointerCookie = XCBQueryPointerCookie(display, win);
    XCBQueryPointer *pointer = XCBQueryPointerReply(display, QueryPointerCookie);

    if(pointer)
    {
        curx = pointer->root_x;
        cury = pointer->root_y;
        nx = pointer->win_x;
        ny = pointer->win_y;
        free(pointer);
    }
    else
    {   return;
    }

    horz = nx < c->w / 2 ? -1 : 1;
    vert = ny < c->h / 2 ? -1 : 1;

    if(horz == -1)
    {
        if(vert == -1)
        {   cur = cursors[CurResizeTopR];
        }
        else
        {   cur = cursors[CurResizeTopL];
        }
    }
    else
    {
        if(vert == -1)
        {   cur = cursors[CurResizeTopR];
        }
        else
        {   cur = cursors[CurResizeTopL];
        }
    }

    XCBCookie GrabPointerCookie = XCBGrabPointerCookie(_wm.dpy, _wm.root, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, None, cur, XCB_CURRENT_TIME);
    XCBGrabPointer *GrabPointer = XCBGrabPointerReply(_wm.dpy, GrabPointerCookie);

    if(!GrabPointer || GrabPointer->status != XCB_GRAB_STATUS_SUCCESS)
    {   free(GrabPointer);
        return;
    }
    oldw = c->w;
    oldh = c->h;
    oldx = c->x;
    oldy = c->y;
    basew = c->minw;
    baseh = c->minh;
    /* Prevent it from being detected as non floating */
    setfloating(c, 1); c->x += 1;
    arrange(c->desktop);
    XCBRaiseWindow(_wm.dpy, win);
    XCBFlush(_wm.dpy);
    XCBGenericEvent *ev = NULL;
    do
    {
        if(ev)
        {
            eventhandler(ev);
            if(XCB_EVENT_RESPONSE_TYPE(ev) == XCB_MOTION_NOTIFY)
            {
                nw = oldw + horz * (((XCBMotionNotifyEvent *)ev)->root_x - curx);
                nh = oldh + vert * (((XCBMotionNotifyEvent *)ev)->root_y - cury);

                nw = MAX(nw, basew);
                nh = MAX(nh, baseh);

                nx = oldx + !~horz * (oldw - nw);
                ny = oldy + !~vert * (oldh - nh);
                resize(c, nx, ny, nw, nh, 1);
                XCBFlush(_wm.dpy);
                DEBUG("(x: %d, y: %d, w: %u, h: %u)", nx, ny, nw, nh);
            }
            /* this accounts for users killing the window (cause they can) */
            else if(XCB_EVENT_RESPONSE_TYPE(ev) == XCB_UNMAP_NOTIFY)
            {   XCBUnMapNotifyEvent *uev = (XCBUnMapNotifyEvent *)ev;
                if(uev->window == win)
                {   free(ev);
                    win = 0;
                    break;
                }
            }
            free(ev);
        }
    } while(_wm.running && !XCBNextEvent(_wm.dpy, &ev) && XCB_EVENT_RESPONSE_TYPE(ev) != XCB_BUTTON_RELEASE);
    XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);
    Monitor *m;
    if(win)
    {
        if ((m = recttomon(c->x, c->y, c->w, c->h)) != _wm.selmon) 
        {
            setclientdesktop(c, m->desksel);
            _wm.selmon = m;
            focus(NULL);
        }
        if(DOCKED(c))
        {   setfloating(c, 0);
        }
    }
    arrange(_wm.selmon->desksel);
    XCBFlush(_wm.dpy);
}

void
SetBorderWidth(const Arg *arg)
{
    const u16 bw = 50;
    if(bw + arg->i < 0)
    {   return;
    }
    const u16 newbw = bw + arg->i;
    Client *c;

    for(c = _wm.selmon->desksel->clients; c; c = nextclient(c))
    {   setborderwidth(c, newbw);
    }
    arrangedesktop(_wm.selmon->desksel);
    XCBFlush(_wm.dpy);
    DEBUG("Border width: [%d]", newbw);
}

void
SetWindowLayout(const Arg *arg)
{
    const Monitor *m = _wm.selmon;
    if(!m) return;
    setdesktoplayout(m->desksel, arg->i);
    arrange(m->desksel);
    XCBFlush(_wm.dpy);
}

void
SetMonitorFact(const Arg *arg)
{
}

void
SpawnWindow(const Arg *arg)
{
    /* TODO: trying to spawn lemonbar fails for some reason? */
    if(!fork())
    {
        struct sigaction sa;
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
    const Monitor *m = _wm.selmon;
    Client *c = m->desksel->sel;
    if(!c)
    {   return;
    }

    const u16 snap = 0;

    /* floating are auto handled to be any window that isnt maxed */
    if(!DOCKED(c))
    {
        if(ISFIXED(c))
        {   /* snap to grid instead. */
            resize(c, m->wx, m->wy, c->w, c->h, 0);
        }
        else
        {   maximize(c);
        }
        DEBUG("Maximized: %u", c->win);
    }
    else /* else its maximized */
    {
        const uint8_t samex = c->x == c->oldx;
        const uint8_t samey = c->y == c->oldy;
        const uint8_t samew = c->w == c->oldw;
        const uint8_t sameh = c->h == c->oldh;
        const uint8_t sameall = samex && samey && samew && sameh;

        if(sameall)
        {
            if(ISFIXED(c))
            {   /* just snap out of grid */
                resize(c, c->x + snap, c->y + snap, c->w, c->h, 0);
            }
            else
            {   /* make half the size */
                resize(c, c->x + WIDTH(c) / 2, c->y + HEIGHT(c) / 2, WIDTH(c) / 2, HEIGHT(c) / 2, 0);
            }
        }
        else
        {   
            if(ISFIXED(c))
            {   /* just use old position */
                resize(c, c->oldx, c->oldy, c->w, c->h, 0);
            }
            else
            {   /* use old dimentions */
                resize(c, c->oldx, c->oldy, c->oldw, c->oldh, 0);
            }
        }
        DEBUG("UnMaximized: %u", c->win);
    }
    arrange(c->desktop);
    XCBFlush(_wm.dpy);
}

void
MaximizeWindowVertical(const Arg *arg) 
{
    /* TODO */
}

void
MaximizeWindowHorizontal(const Arg *arg) 
{
    /* TODO */
}


void
AltTab(const Arg *arg)
{
    static Client *next = NULL;
    if(!_wm.selmon->desksel->clients)
    {   next = NULL;
        return;
    }

    for(Client *c = _wm.selmon->desksel->clients; c; c = nextclient(c))
    {
        DEBUG("%p->", (void *)c);
    }

    if(nextclient(next))
    {   next = nextclient(next);
    }
    else
    {   next = _wm.selmon->desksel->clients;
    }

    Client *tmp = next;

    focus(next);
    next = tmp;
}

void
ToggleStatusBar(const Arg *arg)
{
    const Monitor *m = _wm.selmon;
    if(!m || !m->bar)
    {   return;
    }
    setshowbar(m->bar, !SHOWBAR(m->bar));
    updatebarpos(_wm.selmon);
    XCBMoveResizeWindow(_wm.dpy, _wm.selmon->bar->win, _wm.selmon->wx, _wm.selmon->bar->y, _wm.selmon->ww, _wm.selmon->bar->h);
    arrange(_wm.selmon->desksel);
    XCBFlush(_wm.dpy);
}

void
ToggleFullscreen(const Arg *arg)
{
}
