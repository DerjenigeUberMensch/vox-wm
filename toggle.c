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

/*
 * For people wanting to make new functions:
 * XCB buffers requests to the display (for some of them not all (which is dumb btw.)) so you just do a bunch of stuff then when your done just do XCBFlush();
 */

/* TODO: Make these functions seperate threads */
extern void (*handler[]) (XCBGenericEvent *);
extern WM _wm;
extern CFG _cfg;


static Thread *ct = NULL;

typedef struct ThreadFunction ThreadFunction;
typedef struct Generic Generic;


struct Generic
{
    union
    {
        void *datav;
        void **datavl;
    } v;

    union
    {
        i8 data8[64];
        i16 data16[32];
        i32 data32[16];
        i64 data64[8];

        float dataf[16];
        double datad[8];
        long double datadd[4];  /* compiler specified but should be at most 128 bits */
    } n;
};


struct ThreadFunction
{
    void (*caller)(void *);
    Generic *data;
};

static ThreadFunction ctcalle;
static XCBDisplay *dpy = NULL;

static void *
CurrentFunction(void *unused)
{
    while(1)
    {
        ThreadCondWait(ct, &(ct->cond));
        if(ctcalle.caller)
        {   
            DEBUG("%s", "ran a function");
            ctcalle.caller(ctcalle.data);
            ctcalle.caller = NULL;
            memset(ctcalle.data, 0, sizeof(Generic));
        }
    }
    return NULL;
}


uint8_t
ToggleInit(void)
{
    ct = ThreadCreate(CurrentFunction, NULL);
    ctcalle.data = calloc(1, sizeof(Generic));
    ctcalle.caller = NULL;
    dpy = XCBOpenDisplay(NULL, NULL);
    return !!ct && !!ctcalle.data && !!dpy;
}

void
ToggleExit(void)
{
    ThreadExit(ct);
    free(ctcalle.data);
    XCBCloseDisplay(dpy);
}

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
        killclient(_wm.selmon->desksel->sel->win, Graceful);
    }
}

void
TerminateWindow(const Arg *arg)
{
    if(_wm.selmon->desksel->sel)
    {
        killclient(_wm.selmon->desksel->sel->win, Destroy);
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
    Desktop *desk = _wm.selmon->desksel;
    Client *c = desk->stack;


    arrange(desk);
    XCBFlush(_wm.dpy);
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

