#include "xcb_gtk.h"


static const char *GTK_ATOM_NAMES[GTKLAST] = 
{
    [GTKEdgeConstraints] = "_GTK_EDGE_CONSTRAINTS",
    [GTKShowWindowMenu] = "_GTK_SHOW_WINDOW_MENU",
    [GTKWorkareas] = "_GTK_WORKAREAS",
    [GTKFrameExtents] = "_GTK_FRAME_EXTENTS",
    [GTKThemeVariant] = "_GTK_THEME_VARIANT",
};


static const int GTK_ATOM_LEN[GTKLAST] = 
{
    [GTKEdgeConstraints] = sizeof("_GTK_EDGE_CONSTRAINTS") - 1,
    [GTKShowWindowMenu] = sizeof("_GTK_SHOW_WINDOW_MENU") - 1,
    [GTKWorkareas] = sizeof("_GTK_WORKAREAS") - 1,
    [GTKFrameExtents] = sizeof("_GTK_FRAME_EXTENTS") - 1,
    [GTKThemeVariant] = sizeof("_GTK_THEME_VARIANT") - 1,
};




void
XCBInitGTKAtoms(
        XCBDisplay *display,
        XCBCookie *gtk_cookie_return
        )
{
    unsigned int i;
    for(i = 0; i < GTKLAST; ++i)
    {   gtk_cookie_return[i] = (const XCBCookie) { .sequence = xcb_intern_atom(display, false, GTK_ATOM_LEN[i], GTK_ATOM_NAMES[i]).sequence };
    }
}

void
XCBInitGTKAtomsReply(
        XCBDisplay *display,
        XCBCookie *cookies,
        XCBAtom *atom_return
        )
{
    unsigned int i;
    XCBGenericError *err = NULL;
    xcb_intern_atom_reply_t *rep = NULL;
    for(i = 0; i < GTKLAST; ++i)
    {   
        rep = xcb_intern_atom_reply(display, (xcb_intern_atom_cookie_t) { cookies[i].sequence }, &err);
        if(rep)
        {   atom_return[i] = rep->atom;
        }
        else
        {   atom_return[i] = 0;
        }
        if(err)
        {   
            free(err);
            err = NULL;
        }
        free(rep);
    }
}



