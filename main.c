

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xproto.h>

#include <xcb/xcb_atom.h>
#include "xcb_trl.h"
#include "util.h"



int screen;
XCBDisplay *dpy;



void checkotherwm(void);

void setup(void);
void startup(void);


void xerror(XCBDisplay *display, XCBGenericError *err);

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(err)
    {   debug("An error occured.");
    }
    else
    {   debug("Nothing occured.");
    }
}


void
checkotherwm(void)
{
    XCBGenericEvent *ev = NULL;
    XCBSelectInput(dpy, XCBDefaultRootWindow(dpy, screen), XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
    XCBSync(dpy);  /* XCBFlush has different behaviour suprisingly, its undesired though */
    ev = XCBPollForEvent(dpy);
    if(!ev)
    {   /* assuming this isnt a bug we received nothing because we pinged the server for a response and got nothing */
        /* The other edge case is if the display just doesnt work, however this is covered at startup() if(!dpy) { die(msg); } */
    }
    else    
    {   /* assuming this isnt a bug a "empty" struct is returned with parametors below
         * ev->response_type = 0;
         * ev->sequence = 1;
         * ev->full_sequence = 1;
         */
        free(ev);
        die("FATAL: ANOTHER WINDOW MANAGER IS RUNNING.");
    }
}

void
setup(void)
{
}


void
startup(void)
{
    dpy = XCBOpenDisplay(NULL, &screen);
    if(!dpy)
    {   die("FATAL: UNABLE TO CONNECT TO XSERVER");
    }
    checkotherwm();
    while(1)
    {
        printf("waiting...\n");
        sleep(1);
    }
}

int
main(int argc, char **argv)
{
    startup();
    setup();
    XCBCloseDisplay(dpy);
    return 0;
}
