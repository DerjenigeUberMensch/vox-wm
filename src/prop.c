#include "XCB-TRL/xcb_winutil.h"
#include "main.h"
#include "prop.h"

extern WM _wm;

/* These dont require mutex for the following reasons:
 * - They are stack allocated.
 * - They are set during setup() before propertynotify is intialized.
 * - They are never changed afterwards.
 * - These threads are killed before exit, to prevent the stack from de-initializing these variables.
 */
extern XCBAtom netatom[NetLast];
extern XCBAtom wmatom[WMLast];
extern XCBAtom motifatom;


struct
__PropHandler__
{
    void (*const setup)(XCBDisplay *display, GetPropCookie *cookie);
    XCBCookie (*const get_cookie)(XCBDisplay *display, XCBWindow window);
    void (*const get_reply)(XCBDisplay *display, GetPropCookie *cookie);
};

const static struct 
__PropHandler__ __prophandler__[PropLAST] = 
{                       
    [PropNone] =        { NULL,     NULL,                       NULL                    },
    [PropTransient] =   { NULL,     PropGetTransientCookie,     PropUpdateTrans         },
    [PropWindowState] = { NULL,     PropGetWindowStateCookie,   PropUpdateWindowState   },
    [PropWindowType] =  { NULL,     PropGetWindowTypeCookie,    PropUpdateWindowType    },
    [PropSizeHints] =   { NULL,     PropGetSizeHintsCookie,     PropUpdateSizeHints     },
    [PropWMHints] =     { NULL,     PropGetWMHintsCookie,       PropUpdateWMHints       },
    [PropWMClass] =     { NULL,     PropGetWMClassCookie,       PropUpdateWMClass       },
    [PropWMProtocol] =  { NULL,     PropGetWMProtocolCookie,    PropUpdateWMProtocol    },
    [PropStrut] =       { NULL,     PropGetStrutCookie,         PropUpdateStrut         },
    [PropStrutp] =      { NULL,     PropGetStrutpCookie,        PropUpdateStrutP        },
    [PropNetWMName] =   { NULL,     PropGetNetWMNameCookie,     PropUpdateNetWMName     },
    [PropWMName] =      { NULL,     PropGetWMNameCookie,        PropUpdateWMName        },
    [PropPid] =         { NULL,     PropGetPidCookie,           PropUpdatePid           },
    [PropIcon] =        { NULL,     PropGetIconCookie,          PropUpdateIcon          },
    [PropMotifHints] =  { NULL,     PropGetMotifHintsCookie,    PropUpdateMotifHints    },
    [PropManage] =      { NULL,     NULL,                       PropUpdateManage        },
    [PropUnmanage] =    { NULL,     NULL,                       PropUpdateUnmanage      },

    /* Net setters */

    [PropSetWtype] =    { NULL,     PropGetWindowTypeCookie,    PropUpdateSetWType      },
    [PropUnsetWtype] =  { NULL,     PropGetWindowTypeCookie,    PropUpdateUnsetWType    },
    [PropSetWState] =   { NULL,     PropGetWindowStateCookie,   PropUpdateSetWState     },
    [PropUnsetWState] = { NULL,     PropGetWindowStateCookie,   PropUpdateUnsetWState   },

    /* END */
    [PropExitThread] =  { NULL,     NULL,                       NULL                    },
};


static uint8_t
PropValidType(
    const enum PropertyType type
    )
{
    return (type >= PropNone) & (type < PropLAST);
}


static void
LockMainThread(
        void
        )
{   pthread_mutex_lock(&_wm.mutex);
}

static void
UnlockMainThread(
        void
        )
{   pthread_mutex_unlock(&_wm.mutex);
}

XCBCookie
PropGetInvalidCookie(
        XCBDisplay *display, 
        XCBWindow win
        )
{   
    (void)display;
    (void)win;
    return (XCBCookie) { .sequence = 0 };
}

XCBCookie
PropGetTransientCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetTransientForHintCookie(display, win);
}

XCBCookie
PropGetWindowStateCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

XCBCookie
PropGetWindowTypeCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

XCBCookie
PropGetSizeHintsCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMNormalHintsCookie(display, win);
}

XCBCookie
PropGetWMHintsCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMHintsCookie(display, win);
}

XCBCookie
PropGetWMClassCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMClassCookie(display, win);
}

XCBCookie
PropGetWMProtocolCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMProtocolsCookie(display, win, wmatom[WMProtocols]);
}

XCBCookie
PropGetStrutCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_LENGTH = 4;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
}

XCBCookie
PropGetStrutpCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_P_LENGTH = 12;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
}

XCBCookie
PropGetNetWMNameCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
}

XCBCookie
PropGetWMNameCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
}

XCBCookie
PropGetPidCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetPidCookie(display, win, netatom[NetWMPid]);
}

XCBCookie
PropGetIconCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ANY);
}

XCBCookie
PropGetMotifHintsCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t MOTIF_WM_HINT_LENGTH = 5;
    return XCBGetWindowPropertyCookie(display, win, motifatom, NO_BYTE_OFFSET, MOTIF_WM_HINT_LENGTH, False, motifatom);
}


void
PropUpdateInvalid(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{   
    (void)display;
    (void)cookie;
    (void)cookie->cookie;
}

void
PropUpdateTrans(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindow trans = 0;
    uint8_t transstatus = XCBGetTransientForHintReply(display, cookie->cookie, &trans);
    Client *c;
    Client *ctrans;
    if(transstatus)
    {   
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   
            ctrans = wintoclient(trans);
            /* move to right desktop */
            if(ctrans && ctrans->desktop != c->desktop)
            {   setclientdesktop(c, ctrans->desktop);
            }
            /* set dialog flag(s) */
            if(!ISFLOATING(c))
            {   setfloating(c, 1);
            }
            setwtypedialog(c, 1);
        }
        UnlockMainThread();
    }
}

void
PropUpdateWindowState(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie->cookie);
    Client *c;
    if(prop)
    {
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   clientinitwstate(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

void
PropUpdateWindowType(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie->cookie);
    Client *c;
    if(prop)
    {
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   clientinitwtype(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

void
PropUpdateSizeHints(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBSizeHints hints;
    int hintstatus = XCBGetWMNormalHintsReply(_wm.dpy, cookie->cookie, &hints);
    Client *c;
    if(hintstatus)
    {   
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   updatesizehints(c, &hints);
        }
        UnlockMainThread();
    }
}

void
PropUpdateWMHints(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWMHints *prop = XCBGetWMHintsReply(_wm.dpy, cookie->cookie);
    Client *c;
    u8 wasvisible = 0;
    if(prop)
    {   
        LockMainThread();
        c = wintoclient(cookie->win);
        wasvisible = ISVISIBLE(c);
        if(c)
        {   updatewmhints(c, prop);
        }
        /* arrange if needed, else hide if needed */
        if((wasvisible && !ISVISIBLE(c)) || (!wasvisible && ISVISIBLE(c)))
        {   
            showhide(c);
            arrange(c->desktop);
        }
        UnlockMainThread();
    }
    free(prop);
}

void
PropUpdateWMClass(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWMClass prop;
    int status = XCBGetWMClassReply(_wm.dpy, cookie->cookie, &prop);
    Client *c;
    if(status)
    {
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   updateclass(c, &prop);
        }
        UnlockMainThread();
        XCBWipeGetWMClass(&prop);
    }
}

void
PropUpdateWMProtocol(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWMProtocols prop;
    int status = XCBGetWMProtocolsReply(_wm.dpy, cookie->cookie, &prop);
    Client *c;
    if(status)
    {   
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   updatewindowprotocol(c, &prop);
        }
        UnlockMainThread();
        XCBWipeGetWMProtocols(&prop);
    }
}

void
PropUpdateStrut(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    /* TODO */
    XCBDiscardReply(display, cookie->cookie);
}

void
PropUpdateStrutP(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    /* TODO */
    XCBDiscardReply(display, cookie->cookie);
}

void
PropUpdateNetWMName(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie->cookie);
    char *netname = getnamefromreply(prop);
    Client *c;
    if(prop)
    {
        if(netname)
        {
            LockMainThread();
            c = wintoclient(cookie->win);
            if(c)
            {   updatetitle(c, netname, c->wmname);
            }
            UnlockMainThread();
        }
    }
    free(prop);
}

void
PropUpdateWMName(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie->cookie);
    char *wmname = getnamefromreply(prop);
    Client *c;
    if(prop)
    {
        if(wmname)
        {
            LockMainThread();
            c = wintoclient(cookie->win);
            if(c)
            {   updatetitle(c, c->netwmname, wmname);
            }
            UnlockMainThread();
        }
    }
    free(prop);
}

void
PropUpdatePid(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    const int BAD_PID = -1;
    pid_t prop = XCBGetPidReply(_wm.dpy, cookie->cookie);
    Client *c;
    if(prop != BAD_PID)
    {
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   setclientpid(c, prop);
        }
        UnlockMainThread();
    }
}

void
PropUpdateIcon(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie->cookie);
    uint32_t *icon = geticonprop(prop);
    Client *c;
    if(prop)
    {
        if(icon)
        {
            LockMainThread();
            c = wintoclient(cookie->win);
            if(c)
            {   
                free(c->icon);
                c->icon = icon;
            }
            UnlockMainThread();
        }
    }
    free(prop);
}

void
PropUpdateMotifHints(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(_wm.dpy, cookie->cookie);
    Client *c;
    if(prop)
    {
        LockMainThread();
        c = wintoclient(cookie->win);
        if(c)
        {   updatemotifhints(c, prop);
        }
        UnlockMainThread();
    }
    free(prop);
}

void
PropUpdateManage(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    const XCBWindow win = cookie->win;
    Client *c;
    Client *cf = NULL;     /* client focus */
    XCBCookie requests[ManageClientLAST];
    void *replies[ManageClientLAST];

    managerequest(win, requests);
    managereplies(requests, replies);

    LockMainThread();

    c = manage(win, replies);
    if(c)
    {
        /* Dont waste extra resources if not visible */
        if(ISVISIBLE(c))
        {
            cf = focusrealize(c);
            arrange(c->desktop);
        }
        else
        {   showhide(c);
        }
        setmapstate(c, WMMapStateMapped);
    }
    else if(_wm.selmon->bar && _wm.selmon->bar->win == win)
    {
        cf = focusrealize(NULL);
        arrange(_wm.selmon->desksel);
    }

    XCBMapWindow(_wm.dpy, win);
    focus(cf);

    XCBFlush(_wm.dpy);
    UnlockMainThread();
    managecleanup(replies);
}

void
PropUpdateUnmanage(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    XCBWindow win = cookie->win;
    Client *c;
    Desktop *desk;
    (void)display;
    (void)cookie->cookie;
    LockMainThread();

    c = wintoclient(win);
    if(c)
    {   
        desk = c->desktop;
        unmanage(c, 0);
        /* make sure no dangling pointers */
        c = NULL;
        if(desk->mon->desksel == desk)
        {   
            c = focusrealize(NULL);
            arrange(desk);
        }
        focus(c);
        XCBFlush(_wm.dpy);
    }

    UnlockMainThread();
}

void 
PropUpdateSetWType(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    const XCBWindow win = cookie->win;
    const XCBAtom atom = cookie->arg.ui[0];
    const XCBAtom type = netatom[NetWMWindowType];

    /* TODO: Time based race condition */
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(display, cookie->cookie);

    LockMainThread();

    XCBSetAtomState(display, win, type, atom, prop, 0);
    XCBFlush(display);

    UnlockMainThread();
    free(prop);
}

void 
PropUpdateUnsetWType(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    const XCBWindow win = cookie->win;
    const XCBAtom atom = cookie->arg.ui[0];
    const XCBAtom type = netatom[NetWMWindowType];

    /* TODO: Time based race condition */
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(display, cookie->cookie);

    LockMainThread();

    XCBSetAtomState(display, win, type, atom, prop, 1);
    XCBFlush(display);

    UnlockMainThread();

    free(prop);
}

void 
PropUpdateSetWState(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    const XCBWindow win = cookie->win;
    const XCBAtom atom = cookie->arg.ui[0];
    const XCBAtom type = netatom[NetWMState];

    /* TODO: Time based race condition */
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(display, cookie->cookie);

    LockMainThread();

    XCBSetAtomState(display, win, type, atom, prop, 0);
    XCBFlush(display);

    UnlockMainThread();

    free(prop);

}

void 
PropUpdateUnsetWState(
        XCBDisplay *display, 
        GetPropCookie *cookie
        )
{
    const XCBWindow win = cookie->win;
    const XCBAtom atom = cookie->arg.ui[0];
    const XCBAtom type = netatom[NetWMState];

    /* TODO: Time based race condition */
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(display, cookie->cookie);

    LockMainThread();

    XCBSetAtomState(display, win, type, atom, prop, 1);
    XCBFlush(display);

    UnlockMainThread();

    free(prop);
}

void
PropSetupUpdateProperty(
    XCBDisplay *display,
    GetPropCookie *cookie
    )
{
    const enum PropertyType type = cookie->type;
    const u8 validtype = PropValidType(type);
    if(validtype)
    {
        if(__prophandler__[type].setup)
        {   __prophandler__[type].setup(display, cookie);
        }
    }
}

void
PropUpdateProperty(
        XCBDisplay *display,
        GetPropCookie *cookie
        )
{
    const enum PropertyType type = cookie->type;
    const XCBWindow win = cookie->win;
    const u8 validtype = PropValidType(type);
    u8 valid_client;

    if(validtype)
    {
        cookie->cookie.sequence = 0;
        if(__prophandler__[type].get_cookie)
        {   cookie->cookie = __prophandler__[type].get_cookie(display, win);
        }
        LockMainThread();
        valid_client = cookie->cookie.sequence == 0 || wintoclient(win);
        UnlockMainThread();
        if(valid_client)
        {
            if(__prophandler__[type].get_reply)
            {   __prophandler__[type].get_reply(display, cookie);
            }
        }
        else
        {   XCBDiscardReply(display, cookie->cookie);
        }
    }
}



