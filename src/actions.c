#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sysexits.h>

#include <pthread.h>

#include "XCB-TRL/xcb_image.h"

#include "util.h"
#include "main.h"
#include "floating.h"
#include "bar.h"

/*
 * For people wanting to make new functions:
 * XCB buffers requests to the display (for some of them not all (which is dumb btw.)) so you just do a bunch of stuff then when your done just do XCBFlush();
 */

/* TODO: Make these functions seperate threads */
extern void (*handler[XCBLASTEvent]) (XCBGenericEvent *);
extern WM _wm;
extern UserSettings _cfg;
extern XCBCursor cursors[];

static const char *
bitgravtostr(enum XCBBitGravity gravity)
{
    switch (gravity) 
    {
        case XCBForgetGravity:     return "XCBForgetGravity";
        case XCBNorthWestGravity:  return "XCBNorthWestGravity";
        case XCBNorthGravity:      return "XCBNorthGravity";
        case XCBNorthEastGravity:  return "XCBNorthEastGravity";
        case XCBWestGravity:       return "XCBWestGravity";
        case XCBCenterGravity:     return "XCBCenterGravity";
        case XCBEastGravity:       return "XCBEastGravity";
        case XCBSouthWestGravity:  return "XCBSouthWestGravity";
        case XCBSouthGravity:      return "XCBSouthGravity";
        case XCBSouthEastGravity:  return "XCBSouthEastGravity";
        case XCBStaticGravity:     return "XCBStaticGravity";
        default:                   return "UNKNOWN";
    }
}

void
ActionUserStats(void)
{
    /* PannelCreate(_wm.root, 0, 0, _wm.sw, _wm.sh); */
    Client *c = _wm.selmon->desksel->sel;
    XCBARGB argb;
    if(c)
    {   
        argb.argb = c->bcol;
        (void)argb;

        Debug("========== CLIENT ==========");
        Debug("WINDOW#ID:           %u (0x%08X)", c->win, c->win);
        Debug("NETNAME:             %s", c->netwmname);
        Debug("WMNAME:              %s", c->wmname);
        Debug("CLASS NAME:          %s", c->classname);
        Debug("INSTANCE NAME:       %s", c->instancename);
        Debug("PID:                 %u", c->pid);
        Debug("DESKTOP:             %d", c->desktop ? c->desktop->num : INT16_MIN);
        Debug("EWMH FLAGS:          %u (0x%08X)", c->ewmhflags, c->ewmhflags);
        Debug("FLAGS:               %u (0x%08X)", c->flags, c->flags);

        Debug("");

        Debug("========= GEOMETRY =========");
        Debug("(x: %d, y: %d, w: %u, h: %u)", c->x, c->y, c->w, c->h);
        Debug("(ox: %d, oy: %d, ow: %u, oh: %u)", c->oldx, c->oldy, c->oldw, c->oldh);
        Debug("MIN WIDTH:           %u", c->minw);
        Debug("MIN HEIGHT:          %u", c->minh);
        Debug("MAX WIDTH:           %u", c->maxw);
        Debug("MAX HEIGHT:          %u", c->maxh);
        Debug("WIDTH INCREMENT:     %d", c->incw);
        Debug("HEIGHT INCREMENT:    %d", c->inch);
        Debug("BASE WIDTH:          %u", c->basew);
        Debug("BASE HEIGHT:         %u", c->baseh);
        Debug("MIN ASPECT RATIO:    %f", c->mina);
        Debug("MAX ASPECT RATIO:    %f", c->maxa);
        Debug("GRAVITY:             %s (%u)", bitgravtostr(c->gravity), c->gravity);

        Debug("");

        Debug("========== DECOR ===========");
        Debug("BORDER WIDTH:        %u", c->bw);
        Debug("OLD BORDER WIDTH:    %u", c->oldbw);
        Debug("Icon: (w: %u, h: %u)", c->icon ? c->icon[0] : 0, c->icon ? c->icon[1] : 0);
        Debug("BORDER RGBA: (R: %u, G: %u, B: %u, A: %u)", argb.c.r, argb.c.g, argb.c.b, argb.c.a);

        Debug("");

        Debug("========= WM STATE =========");
        Debug("MODAL:               %s", GET_BOOL(ISMODAL(c)));
        Debug("STICKY:              %s", GET_BOOL(ISSTICKY(c)));
        Debug("MAXIMIZED VERT:      %s", GET_BOOL(ISMAXIMIZEDVERT(c)));
        Debug("MAXIMIZED HORZ:      %s", GET_BOOL(ISMAXIMIZEDHORZ(c)));
        Debug("SHADED:              %s", GET_BOOL(ISSHADED(c)));
        Debug("SKIP TASKBAR:        %s", GET_BOOL(SKIPTASKBAR(c)));
        Debug("SKIP PAGER:          %s", GET_BOOL(SKIPPAGER(c)));
        Debug("HIDDEN:              %s", GET_BOOL(ISHIDDEN(c)));
        Debug("FULLSCREEN:          %s", GET_BOOL(ISFULLSCREEN(c)));
        Debug("ABOVE:               %s", GET_BOOL(ISABOVE(c)));
        Debug("BELOW:               %s", GET_BOOL(ISBELOW(c)));
        Debug("DEMANDS ATTENTION:   %s", GET_BOOL(DEMANDSATTENTION(c)));
        Debug("IS URGENT:           %s", GET_BOOL(ISURGENT(c)));
        Debug("FOCUSED:             %s", GET_BOOL(ISFOCUSED(c)));

        Debug("");

        Debug("======= WM PROTOCOLS =======");
        Debug("WM_TAKE_FOCUS:       %s", GET_BOOL(HASWMTAKEFOCUS(c)));
        Debug("WM_SAVE_YOURSELF:    %s", GET_BOOL(HASWMSAVEYOURSELF(c)));
        Debug("WM_DELETE_WINDOW:    %s", GET_BOOL(HASWMDELETEWINDOW(c)));

        Debug("");

        Debug("====== WM WINDOW TYPE ======");
        Debug("DESKTOP:             %s", GET_BOOL(ISDESKTOP(c)));
        Debug("DOCK:                %s", GET_BOOL(ISDOCK(c)));
        Debug("TOOLBAR:             %s", GET_BOOL(ISTOOLBAR(c)));
        Debug("MENU:                %s", GET_BOOL(ISMENU(c)));
        Debug("UTILITY:             %s", GET_BOOL(ISUTILITY(c)));
        Debug("SPLASH:              %s", GET_BOOL(ISSPLASH(c)));
        Debug("DIALOG:              %s", GET_BOOL(ISDIALOG(c)));
        Debug("DROPDOWN MENU:       %s", GET_BOOL(ISDROPDOWNMENU(c)));
        Debug("POPUP MENU:          %s", GET_BOOL(ISPOPUPMENU(c)));
        Debug("TOOLTIP:             %s", GET_BOOL(ISTOOLTIP(c)));
        Debug("NOTIFICATION:        %s", GET_BOOL(ISNOTIFICATION(c)));
        Debug("COMBO:               %s", GET_BOOL(ISCOMBO(c)));
        Debug("DND:                 %s", GET_BOOL(ISDND(c)));
        Debug("NORMAL:              %s", GET_BOOL(ISNORMAL(c)));

        Debug("");

        Debug("========== EXTRAS ==========");
        Debug("NEVERFOCUS:          %s", GET_BOOL(NEVERFOCUS(c)));
        Debug("MAP ICONIC:          %s", GET_BOOL(ISMAPICONIC(c)));
        Debug("MAP NORMAL:          %s", GET_BOOL(ISMAPNORMAL(c)));
        Debug("FLOATING:            %s", GET_BOOL(ISFLOATING(c)));
        Debug("WAS FLOATING:        %s", GET_BOOL(WASFLOATING(c)));
        Debug("SHOW DECOR:          %s", GET_BOOL(SHOWDECOR(c)));

        Debug("");

        Debug("====== NON-FLAG STATE ======");
        Debug("ISFIXED:             %s", GET_BOOL(ISFIXED(c)));
        Debug("NEVERHOLDFOCUS:      %s", GET_BOOL(NEVERHOLDFOCUS(c)));
        Debug("SHOULDBEFLOATING:    %s", GET_BOOL(SHOULDBEFLOATING(c)));
        Debug("ISSELECTED:          %s", GET_BOOL(ISSELECTED(c)));
        Debug("ISMAPPED:            %s", GET_BOOL(ISMAPPED(c)));
        Debug("ISMAXHORZ:           %s", GET_BOOL(ISMAXHORZ(c)));
        Debug("ISMAXVERT:           %s", GET_BOOL(ISMAXVERT(c)));
        Debug("ISVISIBLE:           %s", GET_BOOL(ISVISIBLE(c)));
        Debug("WSTATENONE:          %s", GET_BOOL(WSTATENONE(c)));

        Debug("");

        Debug("===== NON-FLAG SPECIAL =====");
        Debug("WIDTH:               %u", WIDTH(c));
        Debug("HEIGHT:              %u", HEIGHT(c));
        Debug("OLD WIDTH:           %u", OLDWIDTH(c));
        Debug("OLD HEIGHT:          %u", OLDHEIGHT(c));
        Debug("WASDOCKEDVERT:       %s", GET_BOOL(WASDOCKEDVERT(c)));
        Debug("WASDOCKEDHORZ:       %s", GET_BOOL(WASDOCKEDHORZ(c)));
        Debug("DOCKEDVERT:          %s", GET_BOOL(DOCKEDVERT(c)));
        Debug("DOCKEDHORZ:          %s", GET_BOOL(DOCKEDHORZ(c)));
        Debug("WASDOCKED:           %s", GET_BOOL(WASDOCKED(c)));
        Debug("DOCKED:              %s", GET_BOOL(DOCKED(c)));
        Debug("============================");

        if(c->icon)
        {
            u32 *icon = c->icon;
            u64 i;
            u64 j;
            static XCBWindow win = 0;
            if(win)
            {   XCBDestroyWindow(_wm.dpy, win);
            }
            win = XCBCreateSimpleWindow(_wm.dpy, _wm.root, 0, 0, icon[1] + 5, icon[0] + 5, 0, 0, 0);
            XCBMapWindow(_wm.dpy, win);
            XCBGC gc = XCBCreateGC(_wm.dpy, win, 0, NULL);

            for(i = 0; i < icon[0]; ++i)
            {
                for(j = 0; j < icon[1]; ++j)
                {
                    if(icon[icon[1] * j + i])
                    {
                        XCBSetForeground(_wm.dpy, gc, icon[icon[1] * j + i] & ~((uint32_t)UINT8_MAX << 24));
                        XCBDrawPoint(_wm.dpy, XCB_COORD_MODE_ORIGIN, win, gc, i, j);
                    }
                }
            }
        }

        for(; c; c = nextclient(c))
        {   
            Debug("%s", c->netwmname);
            Debug("%s", c->wmname);
            Debug("");
        }

        Monitor *m;
        for(m = _wm.mons; m; m = nextmonitor(m))
        {
            Debug("mx: %d", m->mx);
            Debug("my: %d", m->my);
            Debug("mw: %d", m->mw);
            Debug("mh: %d", m->mh);
        }
    }
    else
    {   Debug("NULL");
    }
    Debug("Manually flushed win");
    XCBFlush(_wm.dpy);
}

void
ActionStickWindow(Client *c)
{
    /* stanadrd user behaviour if no override */
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(c)
    {   
        setsticky(c, !ISSTICKY(c));
        arrange(c->desktop);

        XCBFlush(_wm.dpy);
    }
}

void
ActionKillWindow(Client *c)
{
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(c)
    {   
        killclient(c, Graceful);

        XCBFlush(_wm.dpy);
    }
}

void
ActionTerminateWindow(Client *c)
{
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(c)
    {   
        killclient(c, Destroy);

        XCBFlush(_wm.dpy);
    }
}

void
ActionSetWindowLayout(Monitor *m, u16 layout)
{
    if(!m)
    {   m = _wm.selmon;
    }

    if(!m) 
    {   return;
    }

    if(layout < LayoutTypeLAST)
    {   DebugLog("Switching to a reserved layout");
    }
    else
    {   DebugLog("Switching to a custom layout");
    }

    setdesktoplayout(m->desksel, layout);
    arrange(m->desksel);

    XCBFlush(_wm.dpy);
}

void
ActionSpawnWindow(char *file_to_run, const char *argv[])
{
    if(!file_to_run)
    {   
        DebugWarn("No file specified to run.");
        return;
    }

    if(!argv || !argv[0])
    {
        DebugWarn("No arguments specified to run.");
        return;
    }

    if(file_to_run != argv[0] && strcmp(file_to_run, argv[0]) != 0)
    {   DebugLog("file_to_run and argv[0] differ, this is not recommeend program startup procedure.");
    }

    int pipefds[2];
    int fd;

    pid_t child;

    if(pipe(pipefds) == -1)
    {   
        DebugWarn("pipe() failed.");
        return;
    }

    int flags = fcntl(pipefds[1], F_GETFD);

    if(flags == -1 || fcntl(pipefds[1], F_SETFD, flags | FD_CLOEXEC) == -1)
    {
        DebugWarn("fcntl() failed.");

        close(pipefds[0]);
        close(pipefds[1]);
        return;
    }

    errno = 0;

    child = fork();

    if(child == -1)
    {
        DebugWarn("fork() failed.");

        close(pipefds[0]);
        close(pipefds[1]);
        return;
    }

    if(child == 0)
    {
        close(pipefds[0]);

        if (_wm.dpy)
        {   close(XCBConnectionNumber(_wm.dpy));
        }

        if(setsid() == -1)
        {   
            int chld_errno = errno;

            int unused = write(pipefds[1], &chld_errno, sizeof(chld_errno));

            (void)unused;

            _exit(EXIT_FAILURE);
        }

        pid_t pid2 = fork();

        /* these 2 if's are to spawn a grandchild and mostly just to prevent vox-wm from
         * being the 'parent' of all sub process, however this is mostly a visual trick,
         * in the process table in linux, as we already detach with close() sigaction etc...
         * so this can fail and we dont care.
         */
        if(pid2 < 0)
        {   DebugWarn("fork() failed preventing child's appearing under the window manager, ignoring...");
        }
        /* exit parent process */
        else if(pid2 > 0)
        {   _exit(EXIT_SUCCESS);
        }

        fd = open("/dev/null", O_RDWR);

        /* replace fd to be banished to the ether, since their buggy. */

        if(fd >= 0)
        {
            (void)dup2(fd, STDIN_FILENO);
            (void)dup2(fd, STDOUT_FILENO);
            (void)dup2(fd, STDERR_FILENO);

            if(fd > STDERR_FILENO)
            {   close(fd);
            }
        }

        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));

        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;

        (void)sigaction(SIGCHLD, &sa, NULL);

        sigset_t empty;

        sigemptyset(&empty);
        (void)sigprocmask(SIG_SETMASK, &empty, NULL);

        /* Some windows can cause us to enter a "Starvation/deadlock" if incorrectly handled, this should prevent that, hopefully */
        /* Refer: https://stackoverflow.com/questions/8319484/regarding-background-processes-using-fork-and-child-processes-in-my-dummy-shel 
         */

        execvp(file_to_run, (char *const *)argv);

        DebugError("execvp() failed.");

        int exec_err = errno;

        fd = write(pipefds[1], &exec_err, sizeof(exec_err));

        /* shut up new compiler warnings */
        if(fd)
        {   (void)fd;
        }

        close(pipefds[1]);

        _exit(EXIT_FAILURE);
    }

    close(pipefds[1]);

    int exec_err = 0;
    ssize_t count;

    do
    {   count = read(pipefds[0], &exec_err, sizeof(exec_err));
    }
    while(count == -1 && errno == EINTR);

    close(pipefds[0]);

    if(count > 0)
    {
        Debug("child's execvp()", strerror(exec_err));

        return;
    }

    if(count == -1)
    {
        DebugError("Failed reading child exec status.");
        return;
    }

#ifdef DEBUG
    int status = 0;
    pid_t wait_result;

    do
    {   wait_result = waitpid(child, &status, WNOHANG );
    }
    while(wait_result == -1 && errno == EINTR);

    if(wait_result == child)
    {
        if(WIFEXITED(status))
        {
            Debug("child exited with %d\n", WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status))
        {
            Debug("child killed by %d\n", WTERMSIG(status));
        }
    }
    else if(wait_result == -1 && errno != ECHILD)
    {   DebugError("WAIT_PID_INTERNAL_ERROR");
    }
#endif

    errno = 0;
}

void
ActionMaximizeWindow(Client *c)
{
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(!c)
    {   return;
    }

    if(!DOCKED(c))
    {   
        
        //setfloating(c, 0);
        maximize(c);
    }
    /* else its maximized */
    else 
    {   
        unmaximize(c);
        setfloating(c, 1);
    }

    Debug("(x: %d, y: %d), (w: %u, h: %u)", c->x, c->y, c->w, c->h);

    arrange(c->desktop);

    XCBFlush(_wm.dpy);
}

void
ActionMaximizeWindowVertical(Client *c) 
{
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(!c)
    {   return;
    }

    if(!DOCKEDVERT(c))
    {   maximizevert(c);
    }
    else
    {   unmaximizevert(c);
    }

    XCBFlush(_wm.dpy);
}

void
ActionMaximizeWindowHorizontal(Client *c) 
{
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(!c)
    {   return;
    }

    if(!DOCKEDHORZ(c))
    {   maximizehorz(c);
    }
    else
    {   unmaximizehorz(c);
    }

    XCBFlush(_wm.dpy);
}

void
ActionRestart(void)
{   
    restarthard();
    quit();
}

void
ActionRestartQ(void)
{
    restart();
    quit();
}

void
ActionQuit(void)
{
    quit();
}

void
ActionToggleStatusBar(Monitor *m)
{
    if(!m)
    {   m = _wm.selmon;
    }

    if(!m)
    {   return;
    }

    Desktop *desk;
    u32 hadbars = 0;

    desk = m->desksel;

    if(!desk)
    {   return;
    }

    Client *c;

    for(c = startstack(desk); c; c = nextstack(c))
    {
        if(ISBAR(c))
        {
            sethidden(c, !ISHIDDEN(c));
            showhide(c);
            hadbars = 1;
        }
    }

    if(hadbars)
    {
        arrange(_wm.selmon->desksel);
        XCBFlush(_wm.dpy);
    }
}

void
ActionToggleFullscreen(Client *c)
{
    if(!c)
    {   c = _wm.selmon->desksel->sel;
    }

    if(!c)
    {   return;
    }
    
    setfullscreen(c, !ISFULLSCREEN(c));

    XCBFlush(_wm.dpy);
}

void
ActionToggleDesktop(Monitor *m, uint16_t index)
{
    if(!m)
    {   m = _wm.selmon;
    }

    setdesktopseli(m, index);
    arrange(m->desksel);
    focus(m->desksel->sel);

    XCBFlush(_wm.dpy);
}
