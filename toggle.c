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
#include "config.h"
#include "toggle.h"
#include "queue.h"

/*
 * For people wanting to make new functions:
 * XCB buffers requests to the display (for some of them not all (which is dumb btw.)) so you just do a bunch of stuff then when your done just do XCBFlush();
 */

/* TODO: Make these functions seperate threads */
extern void (*handler[]) (XCBGenericEvent *);
extern WM _wm;
extern CFG _cfg;

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
    }
}

void
TerminateWindow(const Arg *arg)
{
    if(_wm.selmon->desksel->sel)
    {
        killclient(_wm.selmon->desksel->sel, Destroy);
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
ResizeWindow(const Arg *arg)
{
}

void
SetBorderWidth(const Arg *arg)
{
    if(_cfg.bw + arg->i < 0)
    {   return;
    }
    _cfg.bw += arg->i;
    Client *c;
    for(c = _wm.selmon->desksel->clients; c; c = nextclient(c))
    {   
        XCBSetWindowBorderWidth(_wm.dpy, c->win, _cfg.bw);
        setborderwidth(c, _cfg.bw);
    }
    arrangedesktop(_wm.selmon->desksel);
    XCBFlush(_wm.dpy);
    DEBUG("Border width: [%d]", _cfg.bw);
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
                resize(c, c->x + _cfg.snap, c->y + _cfg.snap, c->w, c->h, 0);
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


void 
View(const Arg *arg)
{
}

