#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

#include <pthread.h>

#include "util.h"
#include "dwm.h"
#include "toggle.h"
#include "keybinds.h"

/*
 * For people wanting to make new functions:
 * XCB buffers requests to the display (for some of them not all (which is dumb btw.)) so you just do a bunch of stuff then when your done just do XCBFlush();
 */

/* TODO: Make these functions seperate threads */
extern void (*handler[XCBLASTEvent]) (XCBGenericEvent *);
extern WM _wm;
extern XCBCursor cursors[CurLast];

static const char *
GET_BOOL(i64 x)
{
    if(x)
    {   return "True";
    }
    else
    {   return "False";
    }
}

void
UserStats(const Arg *arg)
{
    /* PannelCreate(_wm.root, 0, 0, _wm.sw, _wm.sh); */
    Client *c = _wm.selmon->desksel->sel;
    if(c)
    {   
        XCBARGB argb;
        argb.argb = c->bcol;
        DEBUG("(x: %d, y: %d, w: %u, h: %u)", c->x, c->y, c->w, c->h);
        DEBUG("(ox: %d, oy: %d, ow: %u, oh: %u)", c->oldx, c->oldy, c->oldw, c->oldh);
        DEBUG("NETNAME:     %s", c->netwmname);
        DEBUG("WMNAME:      %s", c->wmname);
        DEBUG("CLASSNAME:   %s", c->classname);
        DEBUG("INSTANCENAME:%s", c->instancename);
        DEBUG("WindowID:    %u", c->win);
        DEBUG("PID:         %u", c->pid);
        DEBUG("RGBA:        (R: %u, G: %u, B: %u, A: %u)", argb.c.r, argb.c.g, argb.c.b, argb.c.a);
        DEBUG("BorderWidth: %u", c->bw);
        DEBUG("MINW:        %u", c->minw);
        DEBUG("MINH:        %u", c->minh);
        DEBUG("MAXW:        %u", c->maxw);
        DEBUG("MAXH:        %u", c->maxh);
        DEBUG("INCW:        %d", c->incw);
        DEBUG("INCH:        %d", c->inch);
        DEBUG("Icon:        (w: %u, h: %u)", c->icon ? c->icon[0] : 0, c->icon ? c->icon[1] : 0);

        DEBUG0("STATES.");
        DEBUG("MODAL:               %s", GET_BOOL(ISMODAL(c)));
        DEBUG("STICKY:              %s", GET_BOOL(ISSTICKY(c)));
        DEBUG("MAXIMIZED VERT:      %s", GET_BOOL(ISMAXIMIZEDVERT(c)));
        DEBUG("MAXIMIZED HORZ:      %s", GET_BOOL(ISMAXIMIZEDHORZ(c)));
        DEBUG("SHADED:              %s", GET_BOOL(ISSHADED(c)));
        DEBUG("SKIP TASKBAR:        %s", GET_BOOL(SKIPTASKBAR(c)));
        DEBUG("SKIP PAGER:          %s", GET_BOOL(SKIPPAGER(c)));
        DEBUG("HIDDEN:              %s", GET_BOOL(ISHIDDEN(c)));
        DEBUG("FULLSCREEN:          %s", GET_BOOL(ISFULLSCREEN(c)));
        DEBUG("ABOVE:               %s", GET_BOOL(ISABOVE(c)));
        DEBUG("BELOW:               %s", GET_BOOL(ISBELOW(c)));
        DEBUG("DEMANDS ATTENTION:   %s", GET_BOOL(DEMANDSATTENTION(c)));
        DEBUG("FOCUSED:             %s", GET_BOOL(ISFOCUSED(c)));
        DEBUG0("Supported States.");
        DEBUG("WM_TAKE_FOCUS:       %s", GET_BOOL(HASWMTAKEFOCUS(c)));
        DEBUG("WM_SAVE_YOURSELF:    %s", GET_BOOL(HASWMSAVEYOURSELF(c)));
        DEBUG("WM_DELETE_WINDOW:    %s", GET_BOOL(HASWMDELETEWINDOW(c)));
        DEBUG0("Window Types.");
        DEBUG("DESKTOP:             %s", GET_BOOL(ISDESKTOP(c)));
        DEBUG("DOCK:                %s", GET_BOOL(ISDOCK(c)));
        DEBUG("TOOLBAR:             %s", GET_BOOL(ISTOOLBAR(c)));
        DEBUG("MENU:                %s", GET_BOOL(ISMENU(c)));
        DEBUG("UTILITY:             %s", GET_BOOL(ISUTILITY(c)));
        DEBUG("SPLASH:              %s", GET_BOOL(ISSPLASH(c)));
        DEBUG("DIALOG:              %s", GET_BOOL(ISDIALOG(c)));
        DEBUG("DROPDOWN MENU:       %s", GET_BOOL(ISDROPDOWNMENU(c)));
        DEBUG("POPUP MENU:          %s", GET_BOOL(ISPOPUPMENU(c)));
        DEBUG("TOOLTIP:             %s", GET_BOOL(ISTOOLTIP(c)));
        DEBUG("NOTIFICATION:        %s", GET_BOOL(ISNOTIFICATION(c)));
        DEBUG("COMBO:               %s", GET_BOOL(ISCOMBO(c)));
        DEBUG("DND:                 %s", GET_BOOL(ISDND(c)));
        DEBUG("NORMAL:              %s", GET_BOOL(ISNORMAL(c)));
        DEBUG0("Extras.");
        DEBUG("NEVERFOCUS:          %s", GET_BOOL(NEVERFOCUS(c)));
        DEBUG("MAP ICONIC:          %s", GET_BOOL(ISMAPICONIC(c)));
        DEBUG("FLOATING:            %s", GET_BOOL(ISFLOATING(c)));
        DEBUG("WASFLOATING:         %s", GET_BOOL(WASFLOATING(c)));
    }
    else
    {   DEBUG0("NULL");
    }
    DEBUG0("Manually flushed win");
    XCBFlush(_wm.dpy);
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
    Client *c = _wm.selmon->desksel->sel;

    if(c)
    {
        killclient(c, Graceful);
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
    DEBUG0("Called.");
    static u8 running = 0;
    if(!arg->v || ((XCBButtonPressEvent *)arg->v)->event == _wm.root || running)
    {   return;
    }
    /* get any requests that may have moved the window back */
    XCBSync(_wm.dpy);
    XCBWindow win = ((XCBButtonPressEvent *)arg->v)->event;
    Client *c = wintoclient(win);
    i16 nx, ny;
    i16 x, y;
    i16 oldx, oldy;
    u16 oldw, oldh;
    const XCBCursor cur = cursors[CurMove];
    const i64 detail = ((XCBButtonPressEvent *)arg->v)->detail;
    nx = ny = x = y = oldx = oldy = oldw = oldh = 0;

    XCBCookie GrabPointerCookie = XCBGrabPointerCookie(_wm.dpy, _wm.root, False, MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, None, cur, XCB_CURRENT_TIME);
    XCBGrabPointer *GrabPointer = XCBGrabPointerReply(_wm.dpy, GrabPointerCookie);

    /* FIXME this looks horrible */
    if(GrabPointer)
    {
        if(GrabPointer->status != XCB_GRAB_STATUS_SUCCESS)
        {   free(GrabPointer);
            return;
        }
        free(GrabPointer);
    }
    else
    {   return;
    }

    XCBCookie QueryPointerCookie = XCBQueryPointerCookie(_wm.dpy, win);
    XCBQueryPointer *pointer = XCBQueryPointerReply(_wm.dpy, QueryPointerCookie);

    if(pointer)
    {
        x = pointer->root_x;
        y = pointer->root_y;
        free(pointer);
    }
    else
    {   return;
    }

    if(!c)
    {
        XCBCookie GetGeometryCookie = XCBGetGeometryCookie(_wm.dpy, win);
        XCBGeometry *geom = XCBGetGeometryReply(_wm.dpy, GetGeometryCookie);

        if(geom)
        {
            oldx = geom->x;
            oldy = geom->y;
            oldw = geom->width;
            oldh = geom->height;
            free(geom);
        }
        else
        {   return;
        }
    }
    else
    {   
        oldx = c->x;
        oldy = c->y;
        oldw = c->w;
        oldh = c->h;
    }

    if(c)
    {
        /* prevent DOCKED from computing non floating */
        setfloating(c, 1); c->x += 1;
        arrange(_wm.selmon->desksel);
    }
    else
    {   XCBRaiseWindow(_wm.dpy, win);
    }
    XCBFlush(_wm.dpy);
    XCBGenericEvent *ev = NULL;
    running = 1;
    do
    {
        if(ev)
        {
            eventhandler(ev);
            switch(XCB_EVENT_RESPONSE_TYPE(ev))
            {
                case XCB_MOTION_NOTIFY:
                    nx = oldx + (((XCBMotionNotifyEvent *)ev)->event_x - x);
                    ny = oldy + (((XCBMotionNotifyEvent *)ev)->event_y - y);
                    if(c)
                    {   resize(c, nx, ny, c->w, c->h, 1);
                    }
                    else
                    {   XCBMoveWindow(_wm.dpy, win, nx, ny);
                    }
                    XCBFlush(_wm.dpy);
                    break;
                /* TODO */
                case XCB_BUTTON_PRESS:
                    break;
                case XCB_BUTTON_RELEASE:                                /* failsafe (mainly chromium) */
                    if(((XCBButtonPressEvent *)ev)->detail == detail || ((XCBButtonPressEvent *)ev)->detail == LMB)
                    {   running = 0;
                    }
                    break;
                case XCB_KEY_PRESS:
                    break;
                case XCB_KEY_RELEASE:
                    break;
                /* this accounts for users killing the window (cause they can) */
                case XCB_UNMAP_NOTIFY:
                    if(((XCBUnmapNotifyEvent *)ev)->window == win)
                    {   running = 0;
                    }
                    break;
                case XCB_DESTROY_NOTIFY:
                    if(((XCBDestroyNotifyEvent *)ev)->window == win)
                    {   running = 0;
                    }
                    break;
            }
            free(ev);
        }
    } while(_wm.running && running && !XCBNextEvent(_wm.dpy, &ev));
    running = 0;
    XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);
    Monitor *m;
    c = wintoclient(win);
    if(c)
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
    DEBUG0("Called.");
    static u8 running = 0;
    if(!arg->v || ((XCBButtonPressEvent *)arg->v)->event == _wm.root || running)
    {   return;
    }
    /* get any requests that may have moved the window back */
    XCBSync(_wm.dpy);
    XCBGenericEvent *ev = arg->v;
    const i64 detail = ((XCBButtonPressEvent *)arg->v)->detail;
    XCBWindow win = ((XCBButtonPressEvent *)ev)->event;
    Client *c = wintoclient(win);
    XCBDisplay *display = _wm.dpy;

    i16 curx, cury;
    i32 oldw, oldh;
    i32 nx, ny;
    i32 nw, nh;
    i32 oldx, oldy;
    i8 horz, vert;
    u16 minw, minh;
    u16 maxw, maxh;
    XCBCursor cur;

    /* init data */
    curx = cury = oldw = oldh = nx = ny = nw = nh = oldx = oldy = horz = vert = 0;
    minw = minh = maxw = maxh = 0;

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

    if(!c)
    {
        XCBCookie GetGeometryCookie = XCBGetGeometryCookie(display, win);
        XCBGeometry *wa = XCBGetGeometryReply(display, GetGeometryCookie);

        if(wa)
        {   
            oldw = wa->width;
            oldh = wa->height;
            oldx = wa->x;
            oldy = wa->y;
            free(wa);
        }
        else
        {   return;
        }
    }
    else
    {
        oldw = c->w;
        oldh = c->h;
        oldx = c->x;
        oldy = c->y;
    }

    if(!c)
    {
        XCBSizeHints hints;
        XCBCookie GetWMNormalHintsCookie = XCBGetWMNormalHintsCookie(_wm.dpy, win);
        u8 hintsstatus = XCBGetWMNormalHintsReply(_wm.dpy, GetWMNormalHintsCookie, &hints);
        if(hintsstatus)
        {
            Client c1;
            updatesizehints(&c1, &hints);
            minw = c1.minw;
            minh = c1.minh;
            maxw = c1.maxw;
            maxh = c1.maxh;
        }
        else
        {   return;
        }
    }
    else
    {
        minw = c->minw;
        minh = c->minh;
        maxw = c->maxw;
        maxh = c->maxh;
    }

    const u8 MIN_SIZE = 1 * 1;
    minw = MAX(minw, MIN_SIZE);
    minh = MAX(minh, MIN_SIZE);

    horz = nx < oldw / 2 ? -1 : 1;
    vert = ny < oldh / 2 ? -1 : 1;

    if(horz == -1)
    {
        /* top left */
        if(vert == -1)
        {   cur = cursors[CurResizeTopL];
        }
        /* Bottom Right */
        else
        {   cur = cursors[CurResizeTopR];
        }
    }
    else
    {
        /* top right */
        if(vert == -1)
        {   cur = cursors[CurResizeTopR];
        }
        /* bottom right */
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
    /* Prevent it from being detected as non floating */
    if(c)
    {
        setfloating(c, 1); c->x += 1;
        arrange(c->desktop);
    }
    else
    {   XCBRaiseWindow(display, win);
    }
    XCBFlush(_wm.dpy);
    running = 1;
    ev = NULL;
    do
    {
        if(ev)
        {
            eventhandler(ev);
            switch(XCB_EVENT_RESPONSE_TYPE(ev))
            {   
                case XCB_MOTION_NOTIFY:
                    nw = oldw + horz * (((XCBMotionNotifyEvent *)ev)->root_x - curx);
                    nh = oldh + vert * (((XCBMotionNotifyEvent *)ev)->root_y - cury);

                    if(maxw)
                    {   nw = MIN(nw, maxw);
                    }
                    if(maxh)
                    {   nh = MIN(nh, maxh);
                    }

                    nw = MAX(nw, minw);
                    nh = MAX(nh, minh);

                    nx = oldx + !~horz * (oldw - nw);
                    ny = oldy + !~vert * (oldh - nh);
                    if(c)
                    {   resize(c, nx, ny, nw, nh, 1);
                    }
                    else
                    {   XCBMoveResizeWindow(_wm.dpy, win, nx, ny, nw, nh);
                    }
                    XCBFlush(_wm.dpy);
                    break;
                /* TODO */
                case XCB_BUTTON_PRESS:
                    break;
                case XCB_BUTTON_RELEASE:                                /* failsafe (mainly chromium) */
                    if(((XCBButtonPressEvent *)ev)->detail == detail || ((XCBButtonPressEvent *)ev)->detail == RMB)
                    {   running = 0;
                    }
                    break;
                case XCB_KEY_PRESS:
                    break;
                case XCB_KEY_RELEASE:
                    break;
                /* this accounts for users killing the window (cause they can) */
                case XCB_UNMAP_NOTIFY:
                    if(((XCBUnmapNotifyEvent *)ev)->window == win)
                    {   running = 0;
                    }
                    break;
                case XCB_DESTROY_NOTIFY:
                    if(((XCBDestroyNotifyEvent *)ev)->window == win)
                    {   running = 0;
                    }
                    break;
            }
            free(ev);
        }
    } while(_wm.running && running && !XCBNextEvent(_wm.dpy, &ev)); 
    running = 0;
    XCBUngrabPointer(_wm.dpy, XCB_CURRENT_TIME);
    Monitor *m;
    c = wintoclient(win);
    if(c)
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
SetWindowLayout(const Arg *arg)
{
    const Monitor *m = _wm.selmon;
    if(!m) 
    {   return;
    }
    setdesktoplayout(m->desksel, arg->i);
    arrange(m->desksel);
    XCBFlush(_wm.dpy);
}

void
SpawnWindow(const Arg *arg)
{
    int pipefds[2];
    int count;
    int err;

    pid_t child;
    if(pipe(pipefds))
    {   
        perror("pipe");
        DEBUG0("pipe() failed.");
        err = EX_OSERR;
        return;
    }
    if(fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC))
    {
        perror("fcntl");
        DEBUG0("fcntl() failed.");
        err = EX_OSERR;
        return;
    }
    switch((child = fork()))
    {
        case -1:
            perror("fork");
            DEBUG0("fork() failed.");
            err = EX_OSERR;
            break;
        case 0:
            close(pipefds[0]);
            struct sigaction sa;
            if (_wm.dpy)
                close(XCBConnectionNumber(_wm.dpy));
            setsid();
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sa.sa_handler = SIG_DFL;
            sigaction(SIGCHLD, &sa, NULL);

            /* Some windows can cause us to enter a "Starvation/deadlock" if incorrectly handled, this should prevent that, hopefully */
            /* Refer: https://stackoverflow.com/questions/8319484/regarding-background-processes-using-fork-and-child-processes-in-my-dummy-shel 
            */
            setpgid(0, 0);

            execvp(((char **)arg->v)[0], (char **)arg->v);
            DEBUG0("execvp() failed.");
            write(pipefds[1], &errno, sizeof(int));
            _exit(0);
            break;
        default:
            close(pipefds[1]);
            while ((count = read(pipefds[0], &err, sizeof(errno))) == -1)
            {
                if (errno != EAGAIN && errno != EINTR) 
                {   break;
                }
            }
            if (count) 
            {
                DEBUG("child's execvp(): %s", strerror(err));
                err = EX_UNAVAILABLE;
                return;
            }
            close(pipefds[0]);
            DEBUG0("waiting for child...");
            /* would do 0, over WNOHANG, but as the name implies we cant hang the window manager any time */
            while (waitpid(child, &err, WNOHANG) == -1)
            {
                if (errno != EINTR) 
                {
                    perror("waitpid");
                    DEBUG0("waitpid");
                    err = EX_SOFTWARE;
                    return;
                }
            }
            if (WIFEXITED(err))
            {   DEBUG("child exited with %d\n", WEXITSTATUS(err));
            }
            else if(WIFSIGNALED(err))
            {   DEBUG("child killed by %d\n", WTERMSIG(err));
            }
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
    if(!DOCKED(c))
    {   
        setfloating(c, 0);
        maximize(c);
    }
    else /* else its maximized */
    {   
        unmaximize(c);
        setfloating(c, 1);
    }
    DEBUG("(x: %d, y: %d), (w: %u, h: %u)", c->x, c->y, c->w, c->h);
    arrange(c->desktop);
    XCBFlush(_wm.dpy);
}

void
MaximizeWindowVertical(const Arg *arg) 
{
    const Monitor *m = _wm.selmon;
    Client *c = m->desksel->sel;
    if(!c)
    {   return;
    }
    if(!DOCKEDVERT(c))
    {   maximizevert(c);
    }
    else
    {   unmaximizevert(c);
    }
}

void
MaximizeWindowHorizontal(const Arg *arg) 
{
    const Monitor *m = _wm.selmon;
    Client *c = m->desksel->sel;
    if(!c)
    {   return;
    }
    if(!DOCKEDHORZ(c))
    {   maximizehorz(c);
    }
    else
    {   unmaximizehorz(c);
    }
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
    sethidden(m->bar, !ISHIDDEN(m->bar));
    arrange(_wm.selmon->desksel);
    XCBFlush(_wm.dpy);
}

void
ToggleFullscreen(const Arg *arg)
{
    Client *c = _wm.selmon->desksel->sel;
    if(!c)
    {   return;
    }
    
    setfullscreen(c, !ISFULLSCREEN(c));
}

void
ToggleDesktop(const Arg *arg)
{
    if(!_wm.selmon || !_wm.selmon->desktops)
    {   return;
    }
    u32 index = arg->ui;
    Desktop *desk;
    for(desk = _wm.selmon->desktops; desk; desk = nextdesktop(desk))
    {   
        if(desk->num == index)
        {   
            setdesktopsel(_wm.selmon, desk);
            /* shouldnt need to as clients probably still retained order unless they got corrupted but just in case */
            arrange(desk);
            focus(NULL);
            XCBFlush(_wm.dpy);
            break;
        }
    }
}


