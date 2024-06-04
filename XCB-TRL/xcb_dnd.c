#include "xcb_dnd.h"


static const char *const DND_ATOM_NAMES[XDNDLAST] = 
{
    [XDNDActionAsk] = "XdndActionAsk",
    [XDNDActionCopy] = "XdndActionCopy",
    [XDNDActionLink] = "XdndActionLink",
    [XDNDActionList] = "XdndActionList",
    [XDNDActionMove] = "XdndActionMove",
    [XDNDActionPrivate] = "XdndActionPrivate",
    [XDNDAware] = "XdndAware",
    [XDNDDrop] = "XdndDrop",
    [XDNDEnter] = "XdndEnter",
    [XDNDFinished] = "XdndFinished",
    [XDNDLeave] = "XdndLeave",
    [XDNDPosition] = "XdndPosition",
    [XDNDProxy] = "XdndProxy",
    [XDNDSelection] = "XdndSelection",
    [XDNDStatus] = "XdndStatus",
    [XDNDTypeList] = "XdndTypeList",
};


static const int DND_ATOM_LEN[XDNDLAST] = 
{
    [XDNDActionAsk] = sizeof("XdndActionAsk") - 1,
    [XDNDActionCopy] = sizeof("XdndActionCopy") - 1,
    [XDNDActionLink] = sizeof("XdndActionLink") - 1,
    [XDNDActionList] = sizeof("XdndActionList") - 1,
    [XDNDActionMove] = sizeof("XdndActionMove") - 1,
    [XDNDActionPrivate] = sizeof("XdndActionPrivate") - 1,
    [XDNDAware] = sizeof("XdndAware") - 1,
    [XDNDDrop] = sizeof("XdndDrop") - 1,
    [XDNDEnter] = sizeof("XdndEnter") - 1,
    [XDNDFinished] = sizeof("XdndFinished") - 1,
    [XDNDLeave] = sizeof("XdndLeave") - 1,
    [XDNDPosition] = sizeof("XdndPosition") - 1,
    [XDNDProxy] = sizeof("XdndProxy") - 1,
    [XDNDSelection] = sizeof("XdndSelection") - 1,
    [XDNDStatus] = sizeof("XdndStatus") - 1,
    [XDNDTypeList] = sizeof("XdndTypeList") - 1,
};




void
XCBInitDNDAtomsCookie(
        XCBDisplay *display,
        XCBCookie *cookie_return
        )
{
    unsigned int i;
    for(i = 0; i < XDNDLAST; ++i)
    {   cookie_return[i] = (const XCBCookie) { .sequence = xcb_intern_atom(display, false, DND_ATOM_LEN[i], DND_ATOM_NAMES[i]).sequence };
    }
    
}

void
XCBInitDNDAtomsReply(
        XCBDisplay *display,
        XCBCookie *cookies,
        XCBAtom *atom_return
        )
{
    unsigned int i;
    XCBGenericError *err = NULL;
    xcb_intern_atom_reply_t *rep = NULL;
    for(i = 0; i < XDNDLAST; ++i)
    {   
        rep = xcb_intern_atom_reply(display, (xcb_intern_atom_cookie_t) { cookies[i].sequence }, &err);
        if(rep)
        {   atom_return[i] = rep->atom;
        }
        else
        {   atom_return[i] = 0;
        }
        if(err)
        {   free(err);
        }
        free(rep);
    }
}
