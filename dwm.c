

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>

#include <xcb/xcb_atom.h>
#include "xcb_trl.h"
#include "xcb_winutil.h"
#include "util.h"



#define WM_NAME     "gamer"


int screen;
int running;
int sw;
int sh;
XCBWindow root;
XCBDisplay *dpy;

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];



void checkotherwm(void);
void cleanup(void);

void run(void);
void setup(void);
void sighandler(void);
void startup(void);


void xerror(XCBDisplay *display, XCBGenericError *err);

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
    if((ev = XCBPollForEvent(dpy)))
    {   
        if(ev->response_type == 0) 
        {   
            free(ev);
            XCBCloseDisplay(dpy);
            DIE("%s", "FATAL: ANOTHER WINDOW MANAGER IS RUNNING.");
        }
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
    while(running && ((ev = XCBPollForEvent(dpy)) || XCBNextEvent(dpy, &ev)))
    {
        DEBUG("%d", ev->response_type);
    }
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
    XCBChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8str, 8, XCB_PROP_MODE_REPLACE, (unsigned char *)WM_NAME, strlen(WM_NAME));
    XCBChangeProperty(dpy, root, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&wmcheckwin, 1);
    /* EWMH support per view */
    XCBChangeProperty(dpy, root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom, NetLast);
    XCBDeleteProperty(dpy, root, netatom[NetClientList]);
}

void
sighandler(void)
{
}

void
startup(void)
{
    if(!setlocale(LC_CTYPE, ""))
    {   fputs("WARN: NO_LOCALE_SUPPORT\n", stderr);
    }
    dpy = XCBOpenDisplay(":1", &screen);
    if(!dpy)
    {   DIE("%s", "FATAL: UNABLE TO CONNECT TO XSERVER");
    }
    checkotherwm();
}

int
main(int argc, char **argv)
{
    startup();
    setup();
    XCBCloseDisplay(dpy);
    return 0;
}
