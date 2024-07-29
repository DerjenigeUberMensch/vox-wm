#include <X11/Xatom.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


Display *dpy;
int screen;
int sw;
int sh;
Window win;
Window root;

static void
__test__start_basic(void)
{
    dpy = XOpenDisplay(NULL);
    if(!dpy)
    {   
        printf("Could not load display\n");
        exit(1);
    }
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    

    root = DefaultRootWindow(dpy);

    XSetWindowAttributes wa =
    {
        .bit_gravity = NorthWestGravity,
            ///NorthWestGravity,
        .backing_store = WhenMapped,
    };
    unsigned int mask = CWBitGravity|CWBackingStore;
    win = XCreateWindow(dpy, root, 0, 0, sw, sh, 0, DefaultDepth(dpy, screen), InputOutput, 0, mask, &wa);
    XMapWindow(dpy, win);
}
