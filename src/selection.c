
#include "XCB-TRL/xcb_trl.h"

#include "main.h"
#include "args.h"
#include "util.h"

#include "selection.h"


XCBAtom wmsnatom;
XCBAtom manageratom;
XCBAtom versionatom;
XCBAtom targetsatom;
XCBAtom timestampatom;
XCBAtom atompairatom;
XCBAtom multipleatom;
XCBTimestamp wmsntime = XCB_CURRENT_TIME;

extern WM _wm;
extern XCBAtom netatom[NetLast];

void
checkotherwmsn(void)
{
    const ArgOpt *arg = WMCheckArg(WMArgSkipWMSnCheck);

    enum { BUFF_SIZE = 32 };

    int screencheck = _wm.screen;
    char screenatomname[BUFF_SIZE];
    int status;

    memset(screenatomname, 0, sizeof(screenatomname));

    status = snprintf(screenatomname, BUFF_SIZE - 1, "WM_S%d", screencheck);

    if(unlikely(status < 0))
    {   DIECAT("%s", "FATAL: FAILED TO CREATE SCREEN ATOM. (snprintf)");
    }

    XCBCookie screenatomcookie = XCBInternAtomCookie(_wm.dpy, screenatomname, 0);

    XCBAtom atom = XCBInternAtomReply(_wm.dpy, screenatomcookie);

    if(unlikely(atom == XCBNone))
    {   DIECAT("%s", "FATAL: FAILED TO CREATE SCREEN ATOM. (OutOfMemory)");
    }

    wmsnatom = atom;

    if(arg && arg->found)
    {   return;
    }

    XCBCookie selectioncookie = XCBGetSelectionOwnerCookie(_wm.dpy, atom);

    XCBWindow win = XCBGetSelectionOwnerReply(_wm.dpy, selectioncookie);

    if(unlikely(win != XCBNone))
    {   DIECAT("%s", "FATAL: ANOTHER WINDOW MANAGER IS RUNNING. (SelectionOwner)");
    }
}

void
setupselection(void)
{
    XCBCookie targetscookie = XCBInternAtomCookie(_wm.dpy, "TARGETS", 0);
    XCBCookie timestampcookie = XCBInternAtomCookie(_wm.dpy, "TIMESTAMP", 0);
    XCBCookie versioncookie = XCBInternAtomCookie(_wm.dpy, "VERSION", 0);
    XCBCookie atompaircookie = XCBInternAtomCookie(_wm.dpy, "ATOM_PAIR", 0);
    XCBCookie managercookie = XCBInternAtomCookie(_wm.dpy, "MANAGER", 0);
    XCBCookie multiplecookie = XCBInternAtomCookie(_wm.dpy, "WM_MULTIPLE", 0);

    targetsatom = XCBInternAtomReply(_wm.dpy, targetscookie);
    timestampatom = XCBInternAtomReply(_wm.dpy, timestampcookie);
    atompairatom = XCBInternAtomReply(_wm.dpy, atompaircookie);
    versionatom = XCBInternAtomReply(_wm.dpy, versioncookie);
    manageratom = XCBInternAtomReply(_wm.dpy, managercookie);
    multipleatom = XCBInternAtomReply(_wm.dpy, multiplecookie);
}

int
setupwmsn(XCBEvent *ev, Arg arg)
{
    (void)arg;

    extern XCBAtom wmsnatom;
    extern XCBAtom manageratom;

    if(!_wm.running)
    {   return 0;
    }

    if(XCB_EVENT_SENT(ev) || XCB_EVENT_RESPONSE_TYPE(ev) != XCBPropertyNotify)
    {   return 1;
    }

    XCBPropertyNotifyEvent *pnev = &ev->xproperty;

    if(pnev->window != GET_WM_SN_WINDOW() || pnev->atom != netatom[NetSupportingWMCheck] || pnev->state != XCB_PROPERTY_NEW_VALUE)
    {   return 1;
    }

    XCBSetSelectionOwner(_wm.dpy, wmsnatom, GET_WM_SN_WINDOW(), pnev->time);

    wmsntime = pnev->time;

    XCBCookie cookie = XCBGetSelectionOwnerCookie(_wm.dpy, wmsnatom);

    XCBWindow win = XCBGetSelectionOwnerReply(_wm.dpy, cookie);

    if(win != GET_WM_SN_WINDOW())
    {
        DebugError("Failed to aquire WM_Sn");
        _wm.restart = NoRestart;
        quit();
        return 0;
    }

    XCBEvent gev = {0};
    XCBClientMessageEvent *cmev = &gev.xclient;

    cmev->response_type = XCB_CLIENT_MESSAGE;
    cmev->format = 32;
    cmev->window = _wm.root;
    cmev->type = manageratom;
    cmev->data.data32[0] = pnev->time;
    cmev->data.data32[1] = wmsnatom;
    cmev->data.data32[2] = GET_WM_SN_WINDOW();

    XCBSelectInput(_wm.dpy, GET_WM_SN_WINDOW(), XCBNone);

    XCBSendEvent(_wm.dpy, _wm.root, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char *)cmev);

    XCBFlush(_wm.dpy);

    return 0;
}
