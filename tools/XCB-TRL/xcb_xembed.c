#include "xcb_xembed.h"



static const char *const XEMBED_ATOM_NAMES[XEMBEDLAST] = 
{
    [Xembed] = "_XEMBED"
};


static const unsigned int XEMBED_ATOM_LEN[XEMBEDLAST] =
{
    [Xembed] = sizeof("_XEMBED") - 1
};


void
XCBInitXembedAtomsCookie(
        XCBDisplay *display,
        XCBCookie *xembed_cookie_return
        )
{
    unsigned int i;
    for(i = 0; i < XEMBEDLAST; ++i)
    {   xembed_cookie_return[i] = (const XCBCookie) { .sequence = xcb_intern_atom(display, false, XEMBED_ATOM_LEN[i], XEMBED_ATOM_NAMES[i]).sequence };
    }
}

void
XCBInitXembedAtomsReply(
        XCBDisplay *display,
        XCBCookie *cookies,
        XCBAtom *atom_return
        )
{   
    unsigned int i;
    XCBGenericError *err = NULL;
    xcb_intern_atom_reply_t *rep = NULL;

    for(i = 0; i < XEMBEDLAST; ++i)
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


