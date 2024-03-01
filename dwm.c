

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>

#include <xcb/xcb_atom.h>
#include "xcb_trl.h"
#include "xcb_winutil.h"
#include "util.h"

#include "dwm.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

int screen;
int running;
int sw;
int sh;
XCBWindow root;
XCBDisplay *dpy;

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];

Client baget[256];


void
exithandler(void)
{
    DEBUG("%s", "Process Terminated.");
}

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(err)
    {   DEBUG("%s", "An error occured.");
    }
    else
    {   DEBUG("%s", "Nothing occured.");
    }
}


void
checkotherwm(void)
{
    XCBGenericEvent *ev = NULL;
    XCBSelectInput(dpy, XCBDefaultRootWindow(dpy, screen), XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
    XCBSync(dpy);  /* XCBFlush has different behaviour suprisingly, its undesired though */
    /* XCBPollForEvent calls the XServer itself for the event, So if we get a reply then a type of Window manager must be running */
    if((ev = XCBPollForEvent(dpy)))
    {   
        if(ev->response_type == 0) 
        {   
            free(ev);
            XCBCloseDisplay(dpy);
            DIE("%s", "FATAL: ANOTHER WINDOW MANAGER IS RUNNING.");
        }
        /* UNREACHABLE */
        DIE("%s", "FATAL: UNKNOWN REPONSE_TYPE");
    }
    /* assuming this isnt a bug we received nothing because we pinged the server for a response and got nothing */
    /* The other edge case is if the display just doesnt work, however this is covered at startup() if(!dpy) { DIE(msg); } */
}

void
cleanup(void)
{
}

void 
run(void)
{
    XCBGenericEvent *ev = NULL;
    while(running)
    {
        if(!(ev = XCBPollForQueuedEvent(dpy)))
        {   XCBNextEvent(dpy, &ev);
        }
        switch(ev->response_type)
        {
            case XCB_KEY_PRESS:
                DEBUG("%s", "XCB_KEY_PRESS");
                break;
            case XCB_KEY_RELEASE:
                DEBUG("%s", "XCB_KEY_RELEASE");
                break;
            case XCB_BUTTON_PRESS:
                DEBUG("%s", "XCB_BUTTON_PRESS");
                break;
            case XCB_BUTTON_RELEASE:
                DEBUG("%s", "XCB_BUTTON_RELEASE");
                break;
            case XCB_MOTION_NOTIFY:
                DEBUG("%s", "XCB_MOTION_NOTIFY");
                break;
            case XCB_ENTER_NOTIFY:
                DEBUG("%s", "XCB_ENTER_NOTIFY");
                break;
            case XCB_LEAVE_NOTIFY:
                DEBUG("%s", "XCB_LEAVE_NOTIFY");
                break;
            case XCB_FOCUS_IN :
                DEBUG("%s", "XCB_FOCUS_IN");
                break;
            case XCB_FOCUS_OUT:
                DEBUG("%s", "XCB_FOCUS_OUT");
                break;
            case XCB_KEYMAP_NOTIFY:
                DEBUG("%s", "XCB_KEYMAP_NOTIFY");
                break;
            case XCB_EXPOSE:
                DEBUG("%s", "XCB_EXPOSE");
                break;
            case XCB_GRAPHICS_EXPOSURE:
                DEBUG("%s", "XCB_GRAPHICS_EXPOSURE");
                break;
            case XCB_NO_EXPOSURE:
                DEBUG("%s", "XCB_NO_EXPOSURE");
                break;
            case XCB_VISIBILITY_NOTIFY:
                DEBUG("%s", "XCB_VISIBILITY_NOTIFY");
                break;
            case XCB_CREATE_NOTIFY:
                DEBUG("%s", "XCB_CREATE_NOTIFY");
                break;
            case XCB_DESTROY_NOTIFY:
                DEBUG("%s", "XCB_DESTROY_NOTIFY");
                break;
            case XCB_UNMAP_NOTIFY:
                DEBUG("%s", "XCB_UNMAP_NOTIFY"); 
                break;
            case XCB_MAP_NOTIFY:
                DEBUG("%s", "XCB_MAP_NOTIFY");
                break;
            case XCB_MAP_REQUEST:
                DEBUG("%s", "XCB_MAP_REQUEST");
                break;
            case XCB_REPARENT_NOTIFY:
                DEBUG("%s", "XCB_REPARENT_NOTIFY");
                break;
            case XCB_CONFIGURE_NOTIFY:
                DEBUG("%s", "XCB_CONFIGURE_NOTIFY");
                break;
            case XCB_CONFIGURE_REQUEST:
                DEBUG("%s", "XCB_CONFIGURE_REQUEST");
                break;
            case XCB_GRAVITY_NOTIFY:
                DEBUG("%s", "XCB_GRAVITY_NOTIFY");
                break;
            case XCB_RESIZE_REQUEST: 
                DEBUG("%s", "XCB_RESIZE_REQUEST");
                break;
            case XCB_CIRCULATE_NOTIFY:
                DEBUG("%s", "XCB_CIRCULATE_NOTIFY");
                break;
            case XCB_CIRCULATE_REQUEST:
                DEBUG("%s", "XCB_CIRCULATE_REQUEST");
                break;
            case XCB_PROPERTY_NOTIFY:
                DEBUG("%s", "XCB_PROPERTY_NOTIFY");
                break;
            case XCB_SELECTION_CLEAR:
                DEBUG("%s", "XCB_SELECTION_CLEAR");
                break;
            case XCB_SELECTION_REQUEST:
                DEBUG("%s", "XCB_SELECTION_REQUEST");
                break;
            case XCB_SELECTION_NOTIFY:
                DEBUG("%s", "XCB_SELECTION_NOTIFY");
                break;
            case XCB_COLORMAP_NOTIFY:
                DEBUG("%s", "XCB_COLORMAP_NOTIFY");
                break;
            case XCB_CLIENT_MESSAGE:
                DEBUG("%s", "XCB_CLIENT_MESSAGE");
                break;
            case XCB_MAPPING_NOTIFY:
                DEBUG("%s", "XCB_MAPPING_NOTIFY");
                break;
            case XCB_GE_GENERIC:
                DEBUG("%s", "XCB_GE_GENERIC");
                break;
            case XCB_NONE:
                DEBUG("%s", "AN ERROR OCCURED");
                break;
            default:
                DEBUG("%s[%d]", "UNKNOWN EVENT CODE: ", ev->response_type);
        }
        free(ev);
        ev = NULL;
    }
}

void
scan(void)
{
}

void
setup(void)
{
    /* globals */
    running = 1;

    /* clean up any zombies immediately */
    sighandler();

    sw = XCBDisplayWidth(dpy, screen);
    sh = XCBDisplayHeight(dpy, screen);
    root = XCBRootWindow(dpy, screen);

    const XCBAtomCookie utf8cookie = XCBInternAtomCookie(dpy, "UTF8_STRING", False);
    XCBInitAtoms(dpy, wmatom, netatom);
    const XCBAtom utf8str = XCBInternAtomReply(dpy, utf8cookie);
                            /* simple window */
    XCBWindow wmcheckwin = XCBCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XCBChangeProperty(dpy, wmcheckwin, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&wmcheckwin, 1);
    XCBChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8str, 8, XCB_PROP_MODE_REPLACE, WM_NAME, strlen(WM_NAME));
    XCBChangeProperty(dpy, root, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&wmcheckwin, 1);
    /* EWMH support per view */
    XCBChangeProperty(dpy, root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom, NetLast);
    XCBDeleteProperty(dpy, root, netatom[NetClientList]);

    
    XCBWindowAttributes wa;
    wa.event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|
                    XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_POINTER_MOTION|XCB_EVENT_MASK_ENTER_WINDOW|
                    XCB_EVENT_MASK_LEAVE_WINDOW|XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_PROPERTY_CHANGE;
    XCBChangeWindowAttributes(dpy, root, XCB_CW_EVENT_MASK, &wa);
    XCBSelectInput(dpy, root, wa.event_mask);
}

void
sigchld(int signo) /* signal */
{
    /* wait for childs (zombie proccess) to die */
    (void)signo;
    while (0 < waitpid(-1, NULL, WNOHANG));
}

void
sighandler(void)
{
    if(signal(SIGCHLD, &sigchld) == SIG_ERR)
    {   DIE("%s", "FATAL: CANNOT_INSTALL_SIGCHLD_HANDLER");
    }
    if(signal(SIGTERM, &sigterm) == SIG_ERR) 
    {   
        DIE("%s", "FATAL: CANNOT_INSTALL_SIGTERM_HANDLER");
        signal(SIGHUP, SIG_DFL); /* default signal */
    }
    if(signal(SIGHUP, &sighup) == SIG_ERR) 
    {   
        DEBUG("%s", "WARNING: CANNOT_INSTALL_SIGHUP_HANDLER");
        signal(SIGHUP, SIG_DFL); /* default signal */
    }
    /* wait for zombies to die */
    sigchld(0);
}

void
sighup(int signo) /* signal */
{
    (void)signo;
}

void
sigterm(int signo)
{
    (void)signo;
}

void
startup(void)
{
    if(!setlocale(LC_CTYPE, ""))
    {   fputs("WARN: NO_LOCALE_SUPPORT\n", stderr);
    }
    dpy = XCBOpenDisplay(":1", &screen);
    if(!dpy)
    {   DIE("%s", "FATAL: CANNOT_CONNECT_TO_X_SERVER");
    }
    checkotherwm();
    atexit(exithandler);
}

int
main(int argc, char **argv)
{
    startup();
    setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
    scan();
    run();
    cleanup();
    XCBCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
