#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

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
DragWindow(const Arg *arg) /* movemouse */
{
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
}

void
ToggleFullscreen(const Arg *arg)
{
}
