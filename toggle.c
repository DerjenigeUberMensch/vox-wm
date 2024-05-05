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
#include "xcb_trl.h"
#include "toggle.h"
#include "queue.h"

/*
 * For people wanting to make new functions:
 * XCB buffers requests to the display (for some of them not all (which is dumb btw.)) so you just do a bunch of stuff then when your done just do XCBFlush();
 */

/* TODO: Make these functions seperate threads */
extern void (*handler[]) (XCBGenericEvent *);
extern WM _wm;

void
UserStats(const Arg *arg)
{
    char *str = XCBDebugGetCallStack();
    int i = 0;
    while(str[i])
    {
        fprintf(stderr, "%c", str[i]);
        if(str[i] == '\0')
        {   break;
        }
        ++i;
    }
    free(str);
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
    XCBDisplay *display, 
    XCBWindow win,
    const XCBKeyCode key_or_button)
{
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
MoveWindow(const Arg *arg)
{
}

static void
__ResizeWindow(void *wintoresize)
{
    static XCBDisplay *display = NULL;
    static int screen = 0;

    i16 curx, cury;
    u16 oldw, oldh;
    i16 nx, ny;
    u16 nw, nh;
    i16 oldx, oldy;
    u8 horz, vert;
    u16 basew, baseh;

    /* init data */
    curx = cury = oldw = oldh = nx = ny = nw = nh = oldx = oldy = horz = vert = basew = baseh = 0;

    const u16 framerate = 1000 / (120);

    if(!display)
    {   display = XCBOpenDisplay(NULL, &screen);
        if(!display)
        {   ThreadExit(NULL);
        }
        XCBSelectInput(display, XCBDefaultRootWindow(display, screen), XCB_EVENT_MASK_POINTER_MOTION|XCB_EVENT_MASK_BUTTON_RELEASE);
    }

    if(!wintoresize)
    {   ThreadExit(NULL);
        return;
    }

    const XCBWindow win = *(XCBWindow *)wintoresize;
    free(wintoresize);

    XCBCursor cur = 0;
    XCBTime lasttime = 0;
    u32 pointerstatus = XCB_GRAB_STATUS_FROZEN;
    u8 sizehintstatus = 0;

    XCBCookie QueryPointerCookie = XCBQueryPointerCookie(display, win);
    XCBCookie WindowGeomCookie = XCBGetWindowGeometryCookie(display, win);
    XCBCookie GrabPointerCookie = XCBGrabPointerCookie(display, XCBRootWindow(display, screen), False, MOUSEMASK, 
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCBNone, cur, XCB_CURRENT_TIME);
    XCBCookie SizeHintsCookie = XCBGetWMNormalHintsCookie(display, win);
    
    XCBQueryPointer *pointer = XCBQueryPointerReply(display, QueryPointerCookie);
    XCBGeometry *wingeom = XCBGetWindowGeometryReply(display, WindowGeomCookie);
    XCBGrabPointer *grabpointer = XCBGrabPointerReply(display, GrabPointerCookie);
    XCBSizeHints sizehints; sizehintstatus = XCBGetWMNormalHintsReply(display, SizeHintsCookie, &sizehints);

    if(pointer)
    {
        curx = pointer->root_x;
        cury = pointer->root_y;
        nx = pointer->win_x;
        ny = pointer->win_y;
    }

    if(wingeom)
    {
        horz = nx < (wingeom->width / 2)  ? -1 : 1;
        vert = ny < (wingeom->height / 2) ? -1 : 1;
        oldx = wingeom->x;
        oldy = wingeom->y;
        oldw = wingeom->width;
        oldh = wingeom->height;
    }

    if(grabpointer)
    {   pointerstatus = grabpointer->status;
    }

    if(sizehintstatus)
    {
        if(sizehints.flags & XCB_SIZE_HINT_P_MIN_SIZE)
        {   
            basew = sizehints.min_width;
            baseh = sizehints.min_height;
        }
        else if(sizehints.flags & XCB_SIZE_HINT_P_BASE_SIZE)
        {   
            basew = sizehints.base_width;
            baseh = sizehints.base_height;
        }
    }

    if(!pointer || !wingeom || !grabpointer || !sizehintstatus || pointerstatus != XCB_GRAB_STATUS_SUCCESS)
    {   
        free(pointer);
        free(wingeom);
        if(grabpointer && grabpointer == XCB_GRAB_STATUS_SUCCESS)
        {   XCBUngrabPointer(display, XCB_CURRENT_TIME);
        }
        free(grabpointer);
        ThreadExit(NULL);
        return;
    }

    XCBGenericEvent *ev;
    XCBMotionNotifyEvent *mv;
    do
    {
        if(!XCBNextEvent(display, &ev) || !ev)
        {   break;
        }
        if(XCB_EVENT_RESPONSE_TYPE(ev) == XCB_MOTION_NOTIFY)
        {   
            mv = (XCBMotionNotifyEvent *)ev;
            if(mv->time - lasttime <= framerate)
            {   /* NOP */
            }
            else
            {   
                nw = oldw + horz * (mv->event_x - curx);
                nh = oldh + vert * (mv->event_x - cury);
                /* clamp */
                nw = MAX(nw, basew);
                nh = MAX(nh, baseh);
                /* flip sign if -1 else default to 0 */
                nx = oldx + !~horz * (oldw - nw);
                ny = oldy + !~vert * (oldh - nh);
                XCBMoveResizeWindow(display, win, nx, ny, nw, nh);
            }
            lasttime = mv->time;
        }
    } while(XCB_EVENT_RESPONSE_TYPE(ev) != XCB_BUTTON_RELEASE);
    XCBUngrabPointer(display, XCB_CURRENT_TIME);
    /* cleanup */
    free(pointer);
    free(wingeom);
    free(grabpointer);
    ThreadExit(NULL);
}

void
ResizeWindow(const Arg *arg)
{
    static Thread *t = NULL;
    XCBButtonPressEvent *ev = (XCBButtonPressEvent *)arg->v;
    XCBWindow win = ev->event;
    if(t)
    {   ThreadExit(t);
        t = NULL;
    }
    if(!t)
    {   
        XCBWindow *winarg = malloc(sizeof(XCBWindow));
        if(winarg)
        {   
            *winarg = win;
            t = ThreadCreate(__ResizeWindow, (void *)winarg);
        }
    }
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
    if(!c || !m)
    {   return;
    }

    const u16 snap = 0;

    /* floating are auto handled to be any window that isnt maxed */
    if(ISFLOATING(c))
    {
        if(ISFIXED(c))
        {   /* snap to grid instead. */
            resize(c, m->wx, m->wy, c->w, c->h, 0);
        }
        else
        {   maximize(c);
        }
    }
    else /* else its maximized */
    {
        const uint8_t samex = c->x == c->oldx;
        const uint8_t samey = c->y == c->oldy;
        const uint8_t samew = c->w == c->oldw;
        const uint8_t sameh = c->h == c->oldh;
        const uint8_t sameall = samex & samey & samew & sameh;

        if(sameall)
        {
            if(ISFIXED(c))
            {   /* just snap out of grid */
                resize(c, c->x + snap, c->y + snap, c->w, c->h, 0);
            }
            else
            {   /* make half the size */
                resize(c, c->x + c->w / 2, c->y + c->h / 2, c->w / 2, c->h / 2, 0);
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
    }
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
