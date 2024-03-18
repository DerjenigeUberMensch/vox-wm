#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
#include <xcb/xkb.h>


#include <X11/X.h> /* error codes */

/* keycodes */
#include <X11/keysym.h>

#include "xcb_trl.h"
#include "xcb_winutil.h"
#include "util.h"
#include "events.h"
#include "dwm.h"

#include "config.h"
#include "keybinds.h"



extern void (*handler[]) (XCBGenericEvent *);
WM *_wm = NULL;     /* prevent garbage values */

XCBAtom netatom[NetLast];
XCBAtom wmatom[WMLast];

void
argcvhandler(int argc, char *argv[])
{
    int i;
    for(i = 0; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-h"))
        {
            printf( "Usage: dwm [options]\n"
                    "  -h           Help Information.\n"
                    "  -v           Compiler Information.\n"
                    );
            exit(EXIT_SUCCESS);
        }
        else if (!strcmp(argv[i], "-v"))
        {
            char *compiler = "UNKNOWN";
            int majorversion = -1;
            int minorversion = -1;
            int patchversion = -1;
            #if __GNUC__
            compiler = "GCC";
            majorversion = __GNUC__;
            minorversion = __GNUC_MINOR__;
            patchversion = __GNUC_PATCHLEVEL__;
            #endif
            #if __clang__
            compiler = "CLANG";
            majorversion = __clang_major__;
            minorversion = __clang_minor__;
            patchversion = __clang_patchlevel__;
            #endif
            printf( "Compiling Information.\n"
                    "  Compiled:        %s %s\n"
                    "  Compiler:        [%s v%d.%d.%d]\n" 
                    "  STDC:            [%d] [%lu]\n"
                    "  BYTE_ORDER:      [%d]\n"
                    "  POINTER_SIZE:    [%d]\n"
                    "Version Information.\n"
                    "  VERSION:         [%s]\n"
                    "  MARK:            [%s]\n"
                    "  WM_NAME:         [%s]\n"
                    , 
                    __DATE__, __TIME__,
                    compiler, majorversion, minorversion, patchversion,
                    __STDC_HOSTED__, __STDC_VERSION__,
                    __BYTE_ORDER__,
                    __SIZEOF_POINTER__,
                    VERSION,
                    NAME, 
                    WM_NAME
                    );
            exit(EXIT_SUCCESS);
        }
        else
        {   
            const char exec1 = '.';
            const char exec2 = '/';
            const char execcount = 3; /* not 2 because we need \0 */ /* +1 for the possible 1 letter name and +1 again for \0   */
            if(argv[0] != NULL && strnlen(argv[0], execcount + 2) >= execcount && argv[0][0] == exec1 && argv[0][1] == exec2)
            {   
                /* We can call die because it is very likely this was run manually */
                if(i > 0)
                {
                    printf("%s%s%s", "UNKNOWN COMMAND: '", argv[i], "'\n");
                    printf( "Usage: dwm [options]\n"
                            "  -h           Help Information.\n"
                            "  -v           Compiler Information.\n"
                          );
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {
                /* We dont die because likely this command was run using some form of exec */
                printf("%s%s%s", "UNKNOWN COMMAND: '", argv[i], "'\n");
            }
        }
    }
}


uint8_t
applysizehints(Client *c, int16_t *x, int16_t *y, uint16_t *width, uint16_t *height, uint8_t interact)
{
    int baseismin;
    Monitor *m = c->mon;
    XCBCookie sizehintscookie;
    if(ISFLOATING(c))
    {   sizehintscookie = XCBGetWMNormalHintsCookie(_wm->dpy, c->win);
    }

    /* set minimum possible */
    *width = MAX(1, *width);
    *height = MAX(1, *height);
    if (interact)
    {
        if (*x > _wm->sw) 
        {   *x = _wm->sw - WIDTH(c);
        }
        if (*y > _wm->sh) 
        {   *y = _wm->sh - HEIGHT(c);
        }
        if (*x + *width + (c->bw << 1) < 0)
        {   *x = 0;
        }
        if (*y + *height + (c->bw << 1) < 0)
        {   *y = 0;
        }
    }
    else
    {
        if (*x >= m->wx + m->ww)
        {   *x = m->wx + m->ww - WIDTH(c);
        }
        if (*y >= m->wy + m->wh) 
        {   *y = m->wy + m->wh - HEIGHT(c);
        }
        if (*x + *width + (c->bw << 1) <= m->wx) 
        {   *x = m->wx;
        }
        if (*y + *height + (c->bw << 1) <= m->wy) 
        {   *y = m->wy;
        }
    }
    if (*height < m->bh)
    {   *height = m->bh;
    }
    if (*width  < m->bh)
    {   *width = m->bh;
    }
    if (ISFLOATING(c))
    {
        XCBSizeHints hints;
        u8 status = XCBGetWMNormalHintsReply(_wm->dpy, sizehintscookie, &hints);
        /* On Failure clear flag and ignore hints */
        hints.flags *= !!status;
        updatesizehints(c, &hints);
        /* see last two sentences in ICCCM 4.1.2.3 */
        baseismin = c->basew == c->minw && c->baseh == c->minh;
        /* temporarily remove base dimensions */
        if (!baseismin)
        {
            *width  -= c->basew;
            *height -= c->baseh;
        }
        /* adjust for aspect limits */
        if (c->mina > 0 && c->maxa > 0)
        {
            if (c->maxa < (float)*width / *height) 
            {   *width = *height * c->maxa + 0.5;
            }
            else if (c->mina < (float)*height / *width) 
            {   *height = *width * c->mina + 0.5;
            }
        }
        /* increment calculation requires this */
        if (baseismin)
        {
            *width  -= c->basew;
            *height -= c->baseh;
        }
        /* adjust for increment value */
        if (c->incw)
        {   *width -= *width % c->incw;
        }
        if (c->inch) 
        {   *height -= *height % c->inch;
        }
        /* restore base dimensions */
        *width = MAX(*width + c->basew, c->minw);
        *height = MAX(*height + c->baseh, c->minh);
        if (c->maxw) 
        {   *width = MIN(*width, c->maxw);
        }
        if (c->maxh) 
        {   *height = MIN(*height, c->maxh);
        }
    }
    return *x != c->x || *y != c->y || *width != c->w || *height != c->h;
}

void
arrange(Desktop *desk)
{
}

void
attachbar(Monitor *m, XCBWindow barwin)
{
    /* due to the infrequency of this we can afford multiple cycles wasted here */
    if(!barwin)
    {   
        DEBUG0("Cant Attach no bar to monitor, this shouldnt be possible");
        return;
    }
    if(!m)
    {   
        DEBUG0("Cant Attach barwin because the monitor is NULL, this shouldnt be possible");
        return;
    }
    if(m->barwin)
    {   
        if(m->barwin == barwin)
        {   
            DEBUG0("Cant Attach barwin to same barwin");
        }
        else
        {   
            DEBUG0("Cant Attach barwin because another barwin is already attached");
        }
    }
    else
    {
        DEBUG("Attached barwin: %d", barwin);
        m->barwin = barwin;
        Client *c = manage(barwin);
        if(!c)
        {   DEBUG0("Could not attach bar, for some reason...");
            return;
        }
        sethidden(c, 1);
        setalwaysontop(c, 1);
        setfullscreen(c, 0);
        setneverfocus(c, 1);
        setsticky(c, 1);
        setmodal(c, 1);
        setfixed(c, 1);
    }
}

void
detachbar(Monitor *m)
{
    /* due to the infrequency of this we can afford multiple cycles wasted here */
    if(!m)
    {   DEBUG0("Cant Detach barwin as monitor is NULL");
        return;
    }
    m->barwin = 0;
}

void
attachdesktop(Monitor *m, Desktop *desktop)
{
    desktop->next = m->desktops;
    m->desktops = desktop;
    if(desktop->next)
    {   desktop->next->prev = desktop;
    }
    else
    {  
         /* m->lastdesktop = desktop */
    }
    /* prevent circular linked list */
    desktop->prev = NULL;
    ++m->deskcount;
}

void
detachdesktop(Monitor *m, Desktop *desktop)
{
    Desktop **td;
    for(td = &m->desktops; *td && *td != desktop; td = &(*td)->next);
    *td = desktop->next;
    if(!(*td))
    {
        /* m->lastdesktop = desktop->prev; */
    }
    else if(desktop->next)
    {
        desktop->next->prev = desktop->prev;
    }
    else if(desktop->prev)
    {
        /* m->last = desktop->prev; */
        desktop->prev->next = NULL;
    }
    --m->deskcount;
}

void
attachclient(Client *c)
{
    c->next = c->desktop->clients;
    c->desktop->clients = c;
    if(c->next)
    {   c->next->prev = c;
    }
    else
    {   c->desktop->clast = c;
    }
    /* prevent circular linked list */
    c->prev = NULL;
}

void
detachclient(Client *c)
{
    Client **tc;
    for (tc = &c->desktop->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
    if(!(*tc)) 
    {
        c->desktop->clast = c->prev;
    }
    else if(c->next) 
    {   c->next->prev = c->prev;
    }
    else if(c->prev) 
    {   /* This should be UNREACHABLE but in case we do reach it then this should suffice*/
        c->desktop->clast = c->prev;
        c->prev->next = NULL;
    }
}

u8
checknewbar(XCBWindow win)
{
    u8 status = 0;

    if(_wm->selmon->barwin)
    {   return status;
    }

    XCBCookie ckstrut = XCBGetWindowPropertyCookie(_wm->dpy, win, netatom[NetWMStrut], 0, 4, False, XCB_ATOM_CARDINAL);
    XCBCookie ckstrutp = XCBGetWindowPropertyCookie(_wm->dpy, win, netatom[NetWMStrutPartial], 0, 12, False, XCB_ATOM_CARDINAL);

    XCBWindowProperty *strut = XCBGetWindowPropertyReply(_wm->dpy, ckstrut);
    XCBWindowProperty *strutp = XCBGetWindowPropertyReply(_wm->dpy, ckstrutp);

    if(strut)
    {
        /* https://specifications.freedesktop.org/wm-spec/latest/ 
         * _NET_WM_STRUT */
        u32 *value = XCBGetWindowPropertyValue(strut);
        const u32 left = value[0];
        const u32 right = value[1];
        const u32 top = value[2];
        const u32 bottom = value[3];
        if(left >= _wm->selmon->ww / 2 && right >= _wm->selmon->ww / 2)
        {   
            status = 1;
            if(top && bottom)
            {   /* This is undefined behaviour so just do nothing */
            }
            else if (top)
            {   settopbar(_wm->selmon, 1);
                _wm->selmon->bh = top;
            }
            else if(bottom)
            {   settopbar(_wm->selmon, 0);
                _wm->selmon->bh = bottom;
            }
            else
            {   status = 0;     /* undefined behaviour assume its not a bar */
            }
        }
        free(strut);
    }

    if(strutp)
    {
        if(!status)
        {
            /* https://specifications.freedesktop.org/wm-spec/latest/ 
             * _NET_WM_STRUT_PARTIAL */
            u32 *value = XCBGetWindowPropertyValue(strut);
            const u32 left = value[0];
            const u32 right = value[1];
            const u32 top = value[2];
            const u32 bottom = value[3];
            const u32 left_start_y = value[4];
            const u32 left_end_y = value[5];
            const u32 right_start_y = value[6];
            const u32 right_end_y = value[7];
            const u32 top_start_x = value[8];
            const u32 top_end_x = value[9];
            const u32 bottom_start_x = value[10];
            const u32 bottom_end_x = value[11];

            (void)left_start_y;
            (void)left_end_y;
            (void)right_start_y;
            (void)right_end_y;
            (void)top_start_x;
            (void)top_end_x;
            (void)bottom_start_x;
            (void)bottom_end_x;

            /* while _NET_WM_STRUT_PARTIAL is good in theory in practice it's mostly a pain to implement properly and somewhat computationally expensive 
             * So instead we just use the original _NET_WM_STRUT which worked fine...
             */
            if(left >= _wm->selmon->ww / 2 && right >= _wm->selmon->ww / 2)
            {   
                status = 1;
                if(top && bottom)
                {   /* This is undefined behaviour so just do nothing */
                }
                else if (top)
                {   
                    settopbar(_wm->selmon, 1);
                    _wm->selmon->bh = top;
                }
                else if(bottom)
                {   
                    settopbar(_wm->selmon, 0);
                    _wm->selmon->bh = bottom;
                }
                else
                {   status = 0;     /* undefined behaviour assume its not a bar */
                }
            }
        }
        free(strutp);
    }
    return status;
}

void
checkotherwm(void)
{
    XCBGenericEvent *ev = NULL;
    int response;
    XCBSelectInput(_wm->dpy, XCBRootWindow(_wm->dpy, _wm->screen), XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
    XCBSync(_wm->dpy);  /* XCBFlush has different behaviour suprisingly, its undesired though */
    /* XCBPollForEvent calls the XServer itself for the event, So if we get a reply then a type of Window manager must be running */
    if((ev = XCBPollForEvent(_wm->dpy)))
    {   
        response = ev->response_type;
        free(ev);
        XCBCloseDisplay(_wm->dpy);
        free(_wm);
        if(response == 0) 
        {   DIECAT("%s", "FATAL: ANOTHER WINDOW MANAGER IS RUNNING.");
        }
        /* UNREACHABLE */
        DIECAT("%s", "FATAL: UNKNOWN REPONSE_TYPE");
    }
    /* assuming this isnt a bug we received nothing because we pinged the server for a response and got nothing */
    /* The other edge case is if the display just doesnt work, however this is covered at startup() if(!_wm->dpy) { DIE(msg); } */
}

void
cleanup(void)
{
    XCBCookie cookie;

    cleanupmons();
    cookie = XCBDestroyWindow(_wm->dpy, _wm->wmcheckwin);
    XCBDiscardReply(_wm->dpy, cookie);
    XCBKeySymbolsFree(_wm->syms);
    XCBSync(_wm->dpy);
}

void
cleanupdesktop(Desktop *desk)
{
    Client *c = NULL;
    Client *next = NULL;
    c = desk->clients;
    while(c)
    {   
        next = c->next;
        cleanupclient(c);
        c = next;
    }
    free(desk);
    desk = NULL;
}

void
cleanupclient(Client *c)
{
    detachclient(c);
    free(c);
    c = NULL;
}

void
cleanupmon(Monitor *m)
{
    Desktop *desk = NULL;
    Desktop *desknext = NULL;
    desk = m->desktops;
    while(desk)
    {
        desknext = desk->next;
        cleanupdesktop(desk);
        desk = desknext;
    }
    free(m);
    m = NULL;
}

void
cleanupmons(void)
{
    Monitor *m = NULL;
    Monitor *mnext = NULL;
    m = _wm->mons;
    while(m)
    {   
        mnext = m->next;
        cleanupmon(m);
        m = mnext;
    }
}

void
configure(Client *c)
{
    XCBConfigureNotifyEvent ce;
    ce.response_type = XCB_CONFIGURE_NOTIFY;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above_sibling = XCB_NONE;
    ce.override_redirect = False;
    XCBSendEvent(_wm->dpy, c->win, False, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ce);
}

Desktop *
createdesktop(Monitor *m)
{
    Desktop *desk = calloc(1, sizeof(Desktop));
    if(!desk)
    {
        DEBUG("%s", "WARN: FAILED TO CREATE DESKTOP");
        return NULL;
    }
    desk->layout = _wm->default_layout;
    desk->olayout= _wm->default_layout;
    desk->clients= NULL;
    attachdesktop(m, desk);
    return desk;
}

Client *
createclient(Monitor *m)
{
    Client *c = calloc(1, sizeof(Client ));
    if(!c)
    {   return NULL;
    }
    c->x = c->y = 0;
    c->w = c->h = 0;
    c->oldx = c->oldy = 0;
    c->oldw = c->oldh = 0;
    c->flags = 0;
    c->bw = c->oldbw = 0;
    c->win = 0;
    c->mon = m;
    c->mina = c->maxa = 0;
    c->basew = c->baseh = 0;
    c->incw = c->inch = 0;
    c->maxw = c->maxh = 0;
    c->pid = 0;
    c->desktop = m->desksel;
    return c;
}

Monitor *
createmon(void)
{
    Monitor *m = calloc(1, sizeof(Monitor ));
    if(!m)
    {   /* while calling "DIE" may seem extreme frankly we cannot afford a monitor to fail alloc. */
        DIE("%s", "(OutOfMemory) Could not alloc enough memory for a Monitor");
    }
    m->mx = m->my = 0;
    m->mw = m->mh = 0;
    m->wx = m->wy = 0;
    m->ww = m->wh = 0;
    m->flags = 0;
    m->next = NULL;
    m->barwin = 0;
    m->bx = m->by = 0;
    m->bw = m->bh = 0;
    int i;
    for(i = 0; i < _wm->desktopcount; ++i)
    {   createdesktop(m);
    }
    m->desksel = m->desktops;
    return m;
}

void
exithandler(void)
{   DEBUG("%s", "Process Terminated Successfully.");
}

void
floating(Desktop *desk)
{
}

void
focus(Client *c)
{
    if(!c || !ISVISIBLE(c))
    {   /* selmon should have atleast 1 desktop so no need to check */
        c = nextvisible(_wm->selmon->desksel->clients);
    }
    if(_wm->selmon->desksel->sel && _wm->selmon->desksel->sel != c)
    {   unfocus(_wm->selmon->desksel->sel, 0);
    }
    if(c)
    {   
        if(c->mon != _wm->selmon)
        {   _wm->selmon = c->mon;
        }
        if(ISURGENT(c))
        {   seturgent(c, 0);
        }
        /* make it first on the stack */
        detachclient(c);
        attachclient(c);
        grabbuttons(c->win, 1);
        
        if(_wm->lastfocused && _wm->lastfocused != c)
        {   /* set window border */
        }
        setfocus(c);
    }
    else
    {   
        XCBSetInputFocus(_wm->dpy, _wm->root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm->dpy, _wm->root, netatom[NetActiveWindow]);
    }
    _wm->selmon->desksel->sel = c;
}

i32
getstate(XCBWindow win)
{
    i32 state = 0;
    const XCBCookie cookie = XCBGetWindowPropertyCookie(_wm->dpy, win, wmatom[WMState], 0L, 2L, False, wmatom[WMState]);
    XCBGetWindowAttributes *reply = XCBGetWindowAttributesReply(_wm->dpy, cookie);
    if(reply)
    {
        state = reply->map_state;
        free(reply);
    }
    return state;
}

i8
getrootptr(i16 *x, i16 *y)
{
    u8 samescr;

    XCBCookie cookie = XCBQueryPointerCookie(_wm->dpy, _wm->root);
    XCBQueryPointer *reply = XCBQueryPointerReply(_wm->dpy, cookie);

    if(!reply)
    {   return 0;
    }

    *x = reply->root_x;
    *y = reply->root_y;
    samescr = reply->same_screen;
    free(reply);
    return samescr;
}

void
grabbuttons(XCBWindow win, uint8_t focused)
{
    updatenumlockmask();
    unsigned int i, j;
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm->numlockmask, _wm->numlockmask|XCB_MOD_MASK_LOCK};
    XCBUngrabButton(_wm->dpy, XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY, win);
    /* makesure to ungrab buttons first */
    if (!focused)
    {
        XCBGrabButton(_wm->dpy, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY, win, 0, BUTTONMASK, 
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE);
    }
    for (i = 0; i < LENGTH(buttons); i++)
    {
        if (buttons[i].click == ClkClientWin)
        {
            for (j = 0; j < LENGTH(modifiers); j++)
            {
                XCBGrabButton(_wm->dpy, buttons[i].button, 
                        buttons[i].mask | modifiers[j], 
                        win, 0, BUTTONMASK, 
                        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, 
                        XCB_NONE, XCB_NONE);
            }
        }
    }
}

void
grabkeys(void)
{
    updatenumlockmask();
    u32 i, j, k;
    u32 modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm->numlockmask, _wm->numlockmask|XCB_MOD_MASK_LOCK };
    XCBKeyCode *keycodes[LENGTH(keys)];
    XCBUngrabKey(_wm->dpy, XCB_GRAB_ANY, XCB_MOD_MASK_ANY, _wm->root);
    
    /* This grabs all the keys */
    for(i = 0; i < LENGTH(keys); ++i)
    {   keycodes[i] = XCBKeySymbolsGetKeyCode(_wm->syms, keys[i].keysym);
    }
    for(i = 0; i < LENGTH(keys); ++i)
    {
        for(j = 0; keycodes[i][j] != XCB_NO_SYMBOL; ++j)
        {
            if(keys[i].keysym == XCBKeySymbolsGetKeySym(_wm->syms, keycodes[i][j], 0))
            {   
                for(k = 0; k < LENGTH(modifiers); ++k)
                {
                    XCBGrabKey(_wm->dpy, 
                            keycodes[i][j], keys[i].mod | modifiers[k], 
                            _wm->root, 1, 
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
                }
            }
        }
    }

    for(i = 0; i < LENGTH(keys); ++i)
    {   free(keycodes[i]);
    }
}

void
grid(Desktop *desk)
{
}

Client *
manage(XCBWindow win)
{
    /* checks */
    if(win == _wm->root)
    {   DEBUG("%s", "Cannot manage() root window.");
        return NULL;
    }

    Client *c, *t = NULL;
    XCBWindow trans = 0;
    u8 transstatus = 0;
    XCBWindowGeometry *wg;

    /* get cookies first */

    XCBCookie wgcookie    = XCBGetWindowGeometryCookie(_wm->dpy, win);
    XCBCookie transcookie = XCBGetTransientForHintCookie(_wm->dpy, win);                        
    XCBCookie wtypecookie = XCBGetWindowPropertyCookie(_wm->dpy, win, netatom[NetWMWindowType], 0L, UINT32_MAX, False, XCB_ATOM_ATOM);
    XCBCookie statecookie = XCBGetWindowPropertyCookie(_wm->dpy, win, netatom[NetWMState], 0L, UINT32_MAX, False, XCB_ATOM_ATOM);
    XCBCookie sizehcookie = XCBGetWMNormalHintsCookie(_wm->dpy, win);
    XCBCookie wmhcookie   = XCBGetWMHintsCookie(_wm->dpy, win);

    XCBWindowProperty *wtypeunused;
    XCBWindowProperty *stateunused;
    XCBSizeHints hints;
    u8 hintstatus = 0;
    XCBWMHints *wmh;

    c = createclient(_wm->selmon);
    c->win = win;

    /* wait for replies */
    wg = XCBGetWindowGeometryReply(_wm->dpy, wgcookie);
    transstatus = XCBGetTransientForHintReply(_wm->dpy, transcookie, &trans);
    wtypeunused = XCBGetWindowPropertyReply(_wm->dpy, wtypecookie);
    stateunused = XCBGetWindowPropertyReply(_wm->dpy, statecookie);
    hintstatus = XCBGetWMNormalHintsReply(_wm->dpy, sizehcookie, &hints);
    wmh = XCBGetWMHintsReply(_wm->dpy, wmhcookie);

    /* On Failure clear flag and ignore hints */
    hints.flags *= !!hintstatus;    

    if(wtypeunused)
    {   
        XCBAtom *data = XCBGetPropertyValue(wtypeunused);
        updatewindowtype(c, data, XCBGetPropertyValueLength(wtypeunused, sizeof(XCBAtom)));
    }
    if(stateunused)
    {
        XCBAtom *data = XCBGetPropertyValue(stateunused);
        updatewindowstate(c, data, XCBGetPropertyValueLength(stateunused, sizeof(XCBAtom)));
    }

    if(wg)
    {   /* init geometry */
        c->x = c->oldx = wg->x;
        c->y = c->oldy = wg->y;
        c->w = c->oldw = wg->width;
        c->h = c->oldh = wg->height;
        c->oldbw = wg->border_width;
    }

    if(transstatus && trans && (t = wintoclient(trans)))
    {
        c->mon = t->mon;
        c->desktop = t->desktop;
    }
    else
    {
        c->mon = _wm->selmon;
        /* applyrules() */
    }

    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
    {   c->x = c->mon->wx + c->mon->ww - WIDTH(c);
    }
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
    {   c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
    }
    c->x = MAX(c->x, c->mon->wx);
    c->y = MAX(c->y, c->mon->wy);

    /* get the window type to see if we change the type managed or not */
    /* c->x = CFG_BORDER_WIDTH */
    XCBWindowChanges wc = { .border_width = c->bw };
    XCBConfigureWindow(_wm->dpy, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, &wc);
    /*  XSetWindowBorder(dpy, w, scheme[SchemeBorder][ColBorder].pixel); */
    configure(c);   /* propagates border_width, if size doesn't change */
    updatetitle(c);
    updatesizehints(c, &hints);
    updatewmhints(c, wmh);
    XCBSelectInput(_wm->dpy, win, 
            XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_FOCUS_CHANGE|XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY
            );
    grabbuttons(win, 0);

    if(!ISFLOATING(c))
    {
        /* set both wasfloating and is floating to the same value */
        setfloating(c, trans != XCB_NONE); /* this just covers a few other checks */
        setfloating(c, trans != XCB_NONE || ISALWAYSONTOP(c) || ISFLOATING(c));
    }
    attachclient(c);
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&win, 1);
    setclientstate(c, XCB_WINDOW_NORMAL_STATE);

    if(c->mon == _wm->selmon)
    {   unfocus(_wm->selmon->desksel->sel, 0);
    }
    _wm->selmon->desksel->sel = c;
    XCBMapWindow(_wm->dpy, win);    /* window must be mapped before resizing */
    setfullscreen(c, ISFULLSCREEN(c->mon));
    focus(NULL);


    
    /* reply cleanup */

    free(wmh);
    free(stateunused);
    free(wtypeunused);
    free(wg);

    return c;
}

void
monocle(Desktop *desk)
{
}

Client *
nextclient(Client *c)
{
    return c ? c->next : c;
}

Desktop *
nextdesktop(Desktop *desk)
{
    return desk ? desk->next : desk;
}

Monitor *
nextmonitor(Monitor *m)
{
    return m ? m->next : m;
}

Client *
nextvisible(Client *c)
{
    for(; c && !ISVISIBLE(c); c = c->next);
    return c;
}

Client *
lastvisible(Client *c)
{
    for(; c && !ISVISIBLE(c); c = c->prev);
    return c;
}

void
quit(void)
{
    _wm->running = 0;
}

Monitor *
recttomon(i16 x, i16 y, u16 w, u16 h)
{
	Monitor *m, *r = _wm->selmon;
	int a, area = 0;

	for (m = _wm->mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void
resize(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t interact)
{
    if(applysizehints(c, &x, &y, &width, &height, interact))
    {   resizeclient(c, x, y, width, height);
    }
}

void 
resizeclient(Client *c, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    c->oldx = c->x;
    c->oldy = c->y;
    c->oldw = c->w;
    c->oldh = c->h;
    c->x = x;
    c->y = y;
    c->w = width;
    c->h = height;
    XCBMoveResizeWindow(_wm->dpy, c->win, x, y, width, height);
    XCBSetWindowBorderWidth(_wm->dpy, c->win, c->bw);
    configure(c);
}

void
restack(Desktop *desk)
{
    Client *c;
    XCBGenericEvent *ev;
    XCBWindowChanges wc;

    c = desk->clients;
    if(!c)
    {   return;
    }

    wc.stack_mode = XCB_STACK_MODE_ABOVE;
    wc.sibling = c->mon->barwin;
    /* configure windows */
    while((c = nextclient(c)))
    {   
        XCBConfigureWindow(_wm->dpy, c->win, XCB_CONFIG_WINDOW_SIBLING|XCB_CONFIG_WINDOW_STACK_MODE, &wc);
        wc.sibling = c->win;
    }
}

void
restart(void)
{
    _wm->running = 0;
    _wm->restart = 1;
}

void 
run(void)
{
    XCBGenericEvent *ev = NULL;
    int cleanev = 0;
    XCBSync(_wm->dpy);
    while(_wm->running && ((ev = XCBPollForEvent(_wm->dpy)) || XCBNextEvent(_wm->dpy, &ev)))
    {
        cleanev = XCB_EVENT_RESPONSE_TYPE(ev);
        switch(cleanev)
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
                DEBUG("%s[%d]", "UNKNOWN EVENT CODE: ", cleanev);
        }
        if(handler[cleanev])
        {   handler[cleanev](ev);
        }
        free(ev);
        ev = NULL;
    }
}

/* scan for clients initally */
void
scan(void)
{
    u16 i, num;
    XCBWindow *wins = NULL;
    const XCBCookie cookie = XCBQueryTreeCookie(_wm->dpy, _wm->root);
    XCBQueryTree *tree = NULL;

    if((tree = XCBQueryTreeReply(_wm->dpy, cookie)))
    {
        num = tree->children_len;
        wins = XCBQueryTreeChildren(tree);
        if(wins)
        {
            XCBCookie wa[num];
            XCBCookie tfh[num];
            for(i = 0; i < num; ++i)
            {   
                wa[i] = XCBGetWindowAttributesCookie(_wm->dpy, wins[i]);
                tfh[i] = XCBGetTransientForHintCookie(_wm->dpy, wins[i]);
            }
            
            XCBGetWindowAttributes *replies[num];
            /* filled data no need to free */
            XCBWindow trans[num];
            uint8_t hastrans = 0;
            /* get them replies back */
            for(i = 0; i < num; ++i)
            {
                replies[i] = XCBGetWindowAttributesReply(_wm->dpy, wa[i]);
                hastrans = XCBGetTransientForHintReply(_wm->dpy, tfh[i], &trans[i]);
                trans[i] *= hastrans;

                /* override_redirect only needed to be handled for old windows */
                /* X auto redirects when running wm so no need to do anything else */
                if(!replies[i] || replies[i]->override_redirect || trans[i]) 
                {   continue;
                }
                if(replies[i]->map_state == XCB_MAP_STATE_VIEWABLE ||getstate(wins[i]) == XCB_WINDOW_ICONIC_STATE)
                {   manage(wins[i]);
                }
            }

            /* now the transients */
            for(i = 0; i <  num; ++i)
            {   
                if(replies[i])
                {   
                    if(trans[i] && replies[i]->map_state == XCB_MAP_STATE_VIEWABLE && getstate(wins[i]) == XCB_WINDOW_ICONIC_STATE)
                    {   
                        /* technically we shouldnt have to do this but just in case */
                        if(!wintoclient(wins[i]))
                        {   manage(wins[i]);
                        }
                    }
                    free(replies[i]);
                }
            }
        }
        free(tree);
    }
    else
    {   DEBUG("%s", "Failed to scan for clients.");
    }
}

void
sendmon(Client *c, Monitor *m)
{
    if(c->mon == m)
    {   return;
    }
    unfocus(c, 1);
    detachclient(c);
    c->mon = m;
    c->desktop = m->desksel;
    attachclient(c);
    focus(NULL);
    /* arrangeall() */
}

void
setalwaysontop(Client *c, u8 state)
{
    c->flags &= (~_ALWAYSONTOP);
    c->flags |= (_ALWAYSONTOP * !!state);
}

void
setborderwidth(Client *c, uint16_t border_width)
{
    c->oldbw = c->bw;
    c->bw = border_width;
}

void
setclientdesktop(Client *c, Desktop *desk)
{
    if((c->next || c->prev))
    {   detachclient(c);
    }
    c->desktop = desk;
    attachclient(c);
}

void
setclientstate(Client *c, u8 state)
{
    i32 data[2] = { state, XCB_NONE };
    
    XCBChangeProperty(_wm->dpy, c->win, wmatom[WMState], wmatom[WMState],
            32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}

void
setdesktop(void)
{
    i32 data[1] = { 0 };
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetCurrentDesktop], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 1);
}

void
setdesktopnames(void)
{
    char names[_wm->selmon->deskcount];
    int i;
    for(i = 0; i < _wm->selmon->deskcount; ++i)
    {   names[i] = i;
    }
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetDesktopNames], XCB_ATOM_STRING, 8, XCB_PROP_MODE_REPLACE, names, _wm->selmon->deskcount);
}

void
setdesktopnum(void)
{
    i32 data[1] = { _wm->selmon->deskcount };
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetNumberOfDesktops], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 1);
}

void
setdialog(Client *c, uint8_t state)
{
    c->flags &= (~_DIALOG);
    c->flags |= (_DIALOG * !!state);
}

void
setfixed(Client *c, uint8_t state)
{
    c->flags &= ~(_FIXED);
    c->flags |= (_FIXED * !!state);
}

void
setfloating(Client *c, uint8_t state)
{
    /* set previous floating state */
    c->flags &= (~_WASFLOATING);
    c->flags |= (_WASFLOATING * !!(c->flags & _FLOATING));

    c->flags &= (~_FLOATING);
    c->flags |= (_FLOATING * !!state);
}

void
setfullscreen(Client *c, u8 state)
{
    if(state && !ISFULLSCREEN(c))
    {
        XCBChangeProperty(_wm->dpy, c->win, netatom[NetWMState], XCB_ATOM_ATOM, 32,
        XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom[NetWMStateFullscreen], 1);
        setborderwidth(c, c->bw);
        setborderwidth(c, 0);
        resizeclient(c, c->mon->mx, c->mon->wy, c->mon->mw, c->mon->mh);
        XCBRaiseWindow(_wm->dpy, c->win);
    }
    else if(!state && ISFULLSCREEN(c))
    {
        XCBChangeProperty(_wm->dpy, c->win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
        XCB_PROP_MODE_REPLACE, (unsigned char *)0, 0);
        setborderwidth(c, c->oldbw);
        resizeclient(c, c->oldx, c->oldy, c->oldw, c->oldh);
    }
    c->flags &= (~_FULLSCREEN);
    c->flags |= (_FULLSCREEN * !!state);
}

void
setfocus(Client *c)
{
    if(!NEVERFOCUS(c))
    {
        XCBSetInputFocus(_wm->dpy, c->win, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetActiveWindow], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&(c->win), 1);
        XCBClientMessageEvent ev;
        ev.type = wmatom[WMProtocols];
        ev.response_type = XCB_CLIENT_MESSAGE;
        ev.window = c->win;
        ev.format = 32;
        ev.data.data32[0] = wmatom[WMTakeFocus];
        ev.data.data32[1] = XCB_CURRENT_TIME;
        XCBSendEvent(_wm->dpy, c->win, False, XCB_NONE, (const char *)&ev);
    }
}

void 
sethidden(Client *c, uint8_t state)
{
    c->flags &= (~_HIDDEN);
    c->flags |= (_HIDDEN * !!state);
}
void
setmodal(Client *c, uint8_t state)
{
    c->flags &= (~_MODAL);
    c->flags |= (_MODAL * !!state);
}

void
setneverfocus(Client *c, uint8_t state)
{
    c->flags &= (~_NEVERFOCUS);
    c->flags |= (_NEVERFOCUS * !!state);
}

void 
setshowbar(Monitor *m, uint8_t state)
{
    m->flags &= ~(_SHOWBAR);
    m->flags |= (_SHOWBAR * !!state);
}

void
setsticky(Client *c, u8 sticky)
{
    const XCBWindow win = c->win;
    const XCBAtom replace = !!sticky * netatom[NetWMStateSticky];
    XCBChangeProperty(_wm->dpy, win, netatom[NetWMState], XCB_ATOM_ATOM, 32, 
            XCB_PROP_MODE_REPLACE, (unsigned char *)&replace, !!replace);

    c->flags &= (~_STICKY);
    c->flags |= (_STICKY * !!sticky);
}

void 
settopbar(Monitor *m, uint8_t state)
{
    m->flags &= (~_TOPBAR);
    m->flags |= (_TOPBAR * !!state);
}

void
setup(void)
{
    /* clean up any zombies immediately */
    sighandler();

    /* startup wm */
    _wm->running = 1;
    _wm->desktopcount = 10;
    _wm->syms = XCBKeySymbolsAlloc(_wm->dpy);
    _wm->sw = XCBDisplayWidth(_wm->dpy, _wm->screen);
    _wm->sh = XCBDisplayHeight(_wm->dpy, _wm->screen);
    _wm->root = XCBRootWindow(_wm->dpy, _wm->screen);

    updategeom();
    const XCBCookie utf8cookie = XCBInternAtomCookie(_wm->dpy, "UTF8_STRING", False);
    XCBInitAtoms(_wm->dpy, wmatom, netatom);
    const XCBAtom utf8str = XCBInternAtomReply(_wm->dpy, utf8cookie);
    /* supporting window for NetWMCheck */
    _wm->wmcheckwin = XCBCreateSimpleWindow(_wm->dpy, _wm->root, 0, 0, 1, 1, 0, 0, 0);
    XCBChangeProperty(_wm->dpy, _wm->wmcheckwin, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm->wmcheckwin, 1);
    XCBChangeProperty(_wm->dpy, _wm->wmcheckwin, netatom[NetWMName], utf8str, 8, XCB_PROP_MODE_REPLACE, WM_NAME, LENGTH(WM_NAME));
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetSupportingWMCheck], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&_wm->wmcheckwin, 1);
    /* EWMH support per view */
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetSupported], XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)&netatom, NetLast);
    XCBDeleteProperty(_wm->dpy, _wm->root, netatom[NetClientList]);
    
    setdesktopnum();
    setdesktop();
    setdesktopnames();
    setviewport();

    XCBWindowAttributes wa;
    /* xcb_event_mask_t */
    /* ~0 causes event errors because some event masks override others, for some reason... */
    wa.event_mask = 
                    XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE
                    |XCB_EVENT_MASK_POINTER_MOTION
                    |XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_LEAVE_WINDOW
                    |XCB_EVENT_MASK_STRUCTURE_NOTIFY
                    |XCB_EVENT_MASK_PROPERTY_CHANGE
                    ;   /* the ; is here just so its out of the way */
    XCBChangeWindowAttributes(_wm->dpy, _wm->root, XCB_CW_EVENT_MASK, &wa);
    XCBSelectInput(_wm->dpy, _wm->root, wa.event_mask);
    grabkeys();
    focus(NULL);
}

void
seturgent(Client *c, uint8_t state) 
{
    /* flags stuff */
    c->flags &= (~_URGENT);
    c->flags |= (_URGENT * !!state);

    if(state)
    {   
    }
    else
    {
    }
    /* TODO XXX */
}

void
setviewport(void)
{
    i32 data[2] = { 0, 0 };
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetDesktopViewport], 
            XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
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
    {   DIECAT("%s", "FATAL: CANNOT_INSTALL_SIGCHLD_HANDLER");
    }
    /* wait for zombies to die */
    sigchld(0);
    if(signal(SIGTERM, &sigterm) == SIG_ERR) 
    {   
        DIECAT("%s", "FATAL: CANNOT_INSTALL_SIGTERM_HANDLER");
        signal(SIGTERM, SIG_DFL); /* default signal */
    }
    if(signal(SIGHUP, &sighup) == SIG_ERR) 
    {   
        DEBUG("%s", "WARNING: CANNOT_INSTALL_SIGHUP_HANDLER");
        signal(SIGHUP, SIG_DFL); /* default signal */
    }
    if(signal(SIGINT, &sigterm) == SIG_ERR)
    {   
        DEBUG("%s", "WARNING: CANNOT_INSTALL_SIGINT_HANDLER");
        signal(SIGINT, SIG_DFL);
    }
}

void
sighup(int signo) /* signal */
{
    Restart(NULL);
}

void
sigterm(int signo)
{   
    Quit(NULL);
}

void
startup(void)
{
    _wm = calloc(1, sizeof(WM));
    if(!_wm)
    {   DIECAT("%s", "FATAL: NOT_ENOUGH_MEMORY");
    }
    if(!setlocale(LC_CTYPE, ""))
    {   fputs("WARN: NO_LOCALE_SUPPORT\n", stderr);
    }
    const char *display = ":1";
    _wm->dpy = XCBOpenDisplay(display, &_wm->screen);
    if(!_wm->dpy)
    {   DIECAT("%s", "FATAL: CANNOT_CONNECT_TO_X_SERVER");
    }
    checkotherwm();
    XCBSetErrorHandler(xerror);
    /* This allows for execvp and exec to only spawn process on the specified display rather than the default varaibles */
    if(display)
    {   setenv("DISPLAY", display, 1);
    }
    atexit(exithandler);
}

void
tile(Desktop *desk)
{
    
}

void
unfocus(Client *c, uint8_t setfocus)
{
    if(!c)   
    {   return;
    }
    grabbuttons(c->win, 0);
    _wm->lastfocused = c;
    XCBSetWindowBorderWidth(_wm->dpy, c->win, 0);
    if(setfocus)
    {   
        XCBSetInputFocus(_wm->dpy, _wm->root, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);
        XCBDeleteProperty(_wm->dpy, _wm->root, netatom[NetActiveWindow]);
    }
}

#ifdef XINERAMA
static int
isuniquegeom(XCBXineramaScreenInfo *unique, size_t n, XCBXineramaScreenInfo *info)
{
    while(n--)
    {   if(unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width && unique[n].height == info->height)
        {   return 0;
        }
    }
    return 1;
}
#endif
int
updategeom(void)
{
	int dirty = 0;

#ifdef XINERAMA
    int xienabled;
    int xiactive;
    const XCBQueryExtension *extrep;
    XCBXineramaIsActive *xia;
    int xinerama_screen_number;
    int xid;

    /* check if we even have the extension enabled */
    extrep = xcb_get_extension_data(_wm->dpy, &xid);
    xienabled = !(!extrep || !ext->present);

    if(xienabled)
    {
        xia = xcb_xinerama_is_active(_wm->dpy);
        /* let event handler handle a Xinerama error */
        xcb_xinerama_is_active_reply(_wm->dpy, xia, NULL);
        xiactive = xia && xia->state;
    }
    /* assume no error and proceed */
    if(xiactive)
    {
        int i, j, n, nn;
        Client *c;
        Monitor *m;
        XCBXineramaQueryScreensReply *xsq;
        XCBXineramaScreenInfo *info, *unique = NULL;

        xsq = xcb_xinerama_query_screens_reply(_wm->dpy, xcb_xinerama_query_screens_unchecked(dpy), NULL);

        info = xcb_xinerama_query_screens_screen_info(reply);
        nn = xcb_xinerama_query_screens_screen_info_length(reply);


        for(n = 0, m = _wm->mons; m; m = m->next, ++n);
		/* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(xcb_xinerama_query_screens_reply_t));
        for(i = 0, j = 0; i < nn; ++i)
        {   if(isuniquegeom(unique, j, &info[i]))
            {   memcpy(&unique[j++], &info[i], sizeof(xcb_xinerama_screen_info_t));
            }
        }

        free(xsq);
        nn = j;

		/* new monitors if nn > n */
		for (i = n; i < nn; ++i)
        {
			for (m = _wm->mons; m && m->next; m = m->next);
			if (m)
            {   m->next = createmon();
            }
			else
            {   _wm->mons = createmon();
            }
		}
		for (i = 0, m = _wm->mons; i < nn && m; m = m->next, ++i)
			if (i >= n
			|| unique[i].x_org != m->mx || unique[i].y_org != m->my
			|| unique[i].width != m->mw || unique[i].height != m->mh)
			{
				dirty = 1;
				m->num = i;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
                /* we should update the bar position if we have one */
                updatebarpos(m)
			}
		/* removed monitors if n > nn */
		for (i = nn; i < n; ++i)
        {
			for (m = _wm->mons; m && m->next; m = m->next);
			while ((c = m->desktop->clients)) 
            {
				dirty = 1;
				m->desktop->clients = c->next;
				detachstack(c);
				c->mon = _wm->mons;
				attach(c);
				attachstack(c);
			}
			if (m == selmon)
				selmon = _wm->mons;
			cleanupmon(m);
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{  /* default monitor setup */
		if (!_wm->mons)
        {   _wm->mons = createmon();
        }
		if (_wm->mons->mw != _wm->sw || _wm->mons->mh != _wm->sh) 
        {
			dirty = 1;
			_wm->mons->mw = _wm->mons->ww = _wm->sw;
			_wm->mons->mh = _wm->mons->wh = _wm->sh;
            /* we should update the bar position if we have one */
            updatebarpos(_wm->mons);
		}
	}
	if (dirty) 
    {
		_wm->selmon = _wm->mons;
		_wm->selmon = wintomon(_wm->root);
	}
	return dirty;
}

void
unmanage(Client *c, uint8_t destroyed)
{
    if(!c)
    {   return;
    }
    if(_wm->lastfocused == c)
    {   _wm->lastfocused = NULL;
    }
    if(c->desktop->sel == c)
    {   c->desktop->sel = NULL;
    }
    /* TODO
     * Memory leak if a client is unmaped and maped again
     * (cause we would get the same input focus twice)
     */
    cleanupclient(c);
    updateclientlist();
    focus(NULL);
}

void
updatebarpos(Monitor *m)
{
    m->wy = m->my;
    m->wh = m->mh;

    if(SHOWBAR(m))
    {
        m->wh -= m->bh;
        if(TOPBAR(m))
        {
            m->by = m->wy;
            m->wy+= m->bh;
        }
        else
        {   
            m->by = m->wy + m->wh;
        }
    }
    else
    {
        m->by = -m->bh;
    }
}

void
updateclientlist(void)
{
    Client *c;
    Monitor *m;
    Desktop *desk;
    
    XCBDeleteProperty(_wm->dpy, _wm->root, netatom[NetClientList]);

    m = _wm->mons;
    while((m = nextmonitor(m)))
    {
        desk = m->desktops;
        while((desk = nextdesktop(desk)))
        {
            c = desk->clients;
            while((c = nextclient(c)))
            {   XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetClientList], XCB_ATOM_WINDOW, 32, XCB_PROP_MODE_APPEND, (unsigned char *)&(c->win), 1);
            }
        }
    }
}

void
updatedesktop(void)
{
    i32 data[1] = { _wm->selmon->desksel->num };
    XCBChangeProperty(_wm->dpy, _wm->root, netatom[NetCurrentDesktop], XCB_ATOM_CARDINAL, 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 1);
}


void
updatenumlockmask(void)
{
    /* taken from i3 */
    XCBKeyboardModifier *reply;
    XCBGenericError *err;

    reply = xcb_get_modifier_mapping_reply(_wm->dpy, xcb_get_modifier_mapping(_wm->dpy), &err);
    if(err)
    {   return;
    }

	xcb_keycode_t *codes = xcb_get_modifier_mapping_keycodes(reply);
	xcb_keycode_t target, *temp = NULL;
	unsigned int i, j;

    if(!(temp = xcb_key_symbols_get_keycode(_wm->syms, XK_Num_Lock)))
    {   free(reply);
        return;
    }

	target = *temp;
	free(temp);

	for(i = 0; i < 8; i++)
		for(j = 0; j < reply->keycodes_per_modifier; j++)
			if(codes[i * reply->keycodes_per_modifier + j] == target)
				_wm->numlockmask = (1 << i);
    free(reply);
}

void
updatesizehints(Client *c, XCBSizeHints *size)
{
    /* init values */
    c->basew = c->baseh = 0;
    c->incw = c->inch = 0;
    c->maxw = c->maxh = 0;
    c->minw = c->minh = 0;
    c->maxa = c->mina = 0.0;

    /* size is uninitialized, ensure that size.flags aren't used */
    size->flags += !size->flags * XCB_SIZE_HINT_P_SIZE;

    if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        c->minw = size->min_width;
        c->minh = size->min_height;
    }
    else if(size->flags & XCB_SIZE_HINT_P_BASE_SIZE)
    {
        c->minw = size->base_width;
        c->minh = size->base_height;
    }

    if(size->flags & XCB_SIZE_HINT_P_BASE_SIZE)
    {
        c->basew = size->base_width;
        c->baseh = size->base_height;
    }
    else if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        c->basew = c->minw;
        c->baseh = c->minh;
    }

    if(size->flags & XCB_SIZE_HINT_P_RESIZE_INC)
    {
        c->incw = size->width_inc;
        c->inch = size->height_inc;
    }
    if(size->flags & XCB_SIZE_HINT_P_MIN_SIZE)
    {
        c->maxw = size->max_width;
        c->maxh = size->max_height;
    }
    if(size->flags & XCB_SIZE_HINT_P_ASPECT)
    {
        c->mina = (float)size->min_aspect_den / size->min_aspect_num;
        c->maxa = (float)size->max_aspect_num / size->max_aspect_den;
    }

    if((c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh))
    {   setfixed(c, 1);
    }
}

void
updatetitle(Client *c)
{
}

void
updatewindowstate(Client *c, XCBAtom states[], uint32_t atomslength)
{
    if(!states || !c)
    {   return;
    }

    Monitor *m = c->mon;
    u32 i;
    XCBAtom state = 0;
    for(i = 0; i < atomslength; ++i)
    {
        state = states[i];
        /* This is similiar to those Windows 10 dialog boxes that play the err sound and cant click anything else */
        if (state == netatom[NetWMStateModal])
        {
            if(c->mon->desksel->sel != c)
            {   focus(c);
            }
        }
        else if (state == netatom[NetWMStateAbove] || state == netatom[NetWMStateAlwaysOnTop])
        {
            setalwaysontop(c, 1);
        }
        else if (state == netatom[NetWMStateDemandAttention])
        {
            seturgent(c, 1);
        }
        else if (state == netatom[NetWMStateFullscreen])
        {
            setfullscreen(c, 1);
        }
        else if (state == netatom[NetWMStateMaximizedHorz])
        {
            resize(c, c->x, c->mon->wy, c->w, c->mon->wh, 0);
        }
        else if (state == netatom[NetWMStateMaximizedVert])
        {
            resize(c, c->mon->wx, c->y, c->mon->ww, c->h, 0);
        }
        else if (state == netatom[NetWMStateSticky])
        {
            setsticky(c, 1);
        }
        else if (state == netatom[NetWMStateBelow])
        {   /* attach last */
            XCBLowerWindow(_wm->dpy, c->win);
        }
        else if (state == netatom[NetWMStateSkipTaskbar])
        {   
        }
        else if (state == netatom[NetWMStateSkipPager])
        {
        }
        else if (state == netatom[NetWMStateHidden])
        {   sethidden(c, 1);
        }
        else if (state == netatom[NetWMStateFocused])
        {
            if(c->desktop->sel != c)
            {   focus(c);
            }
        }
        else if (state == netatom[NetWMStateShaded])
        {
        }
    }
}

void
updatewindowtype(Client *c, XCBAtom wtypes[], uint32_t atomslength)
{
    if(!wtypes || !c)
    {   return;
    }
    Monitor *m = c->mon;
    XCBWindow win = c->win;
    XCBAtom wtype = 0;
    i32 i;
    for(i = 0; i < atomslength; ++i)
    {
        wtype = wtypes[i];
        if (wtype == netatom[NetWMWindowTypeDesktop])
        {   
            setneverfocus(c, 1);
            /* TODO */
        }
        else if (wtype == netatom[NetWMWindowTypeDock])
        {
            /* doesnt work */
            if(checknewbar(c->win))
            {
                attachbar(m, c->win);
                setshowbar(m, 1);
                updatebarpos(m);
            }
        }
        else if (wtype == netatom[NetWMWindowTypeToolbar])
        {   /* TODO */
        }
        else if (wtype == netatom[NetWMWindowTypeMenu])
        {   /* TODO */
        }
        else if (wtype == netatom[NetWMWindowTypeUtility])
        {   /* TODO */
        }
        else if (wtype == netatom[NetWMWindowTypeSplash])
        {   /* IGNORE */
        }
        else if (wtype == netatom[NetWMWindowTypeDialog])
        {   setdialog(c, 1);
        }
        else if (wtype == netatom[NetWMWindowTypeDropdownMenu])
        {   setdialog(c, 1);
        }
        else if (wtype == netatom[NetWMWindowTypePopupMenu])
        {   /* override-redirect IGNORE */
        }
        else if (wtype == netatom[NetWMWindowTypeTooltip])
        {   /* override-redirect IGNORE */
        }
        else if (wtype == netatom[NetWMWindowTypeNotification])
        {   /* override-redirect IGNORE */
        }
        else if (wtype == netatom[NetWMWindowTypeCombo])
        {   /* override-redirect IGNORE */
        }
        else if (wtype == netatom[NetWMWindowTypeDnd])
        {   /* override-redirect IGNORE */
        }
        else if (wtype == netatom[NetWMWindowTypeNormal])
        {   /* This hint indicates that this window has no special properties IGNORE */
        }
    }
    DEBUG("%d", i);
}

void
updatewmhints(Client *c, XCBWMHints *wmh)
{
    if(wmh)
    {
        if(c == c->desktop->sel && wmh->flags & XCB_WM_HINT_URGENCY)
        {
            wmh->flags &= ~XCB_WM_HINT_URGENCY;
            XCBSetWMHintsCookie(_wm->dpy, c->win, wmh);
        }
        else
        {   seturgent(c, !!(wmh->flags & XCB_WM_HINT_URGENCY));
        }
        if(wmh->flags & XCB_WM_HINT_INPUT)
        {   setneverfocus(c, !wmh->input);
        }
        else
        {   setneverfocus(c, 0);
        }
    }
}

void
winsetstate(XCBWindow win, i32 state)
{
    i32 data[] = { state, XCB_NONE };
    XCBChangeProperty(_wm->dpy, win, wmatom[WMState], wmatom[WMState], 32, XCB_PROP_MODE_REPLACE, (unsigned char *)data, 2);
}

Client *
wintoclient(XCBWindow win)
{
    Client *c = NULL;
    Desktop *desk = NULL;
    Monitor *m = NULL;

    for(m = _wm->mons; m; m = m->next)
    {
        for(desk = m->desktops; desk; desk = desk->next)
        {
            for(c = desk->clients; c; c = c->next)
            {
                if(c->win == win)
                {
                    return c;
                }
            }
        }
    }
    return c;
}

Monitor *
wintomon(XCBWindow win)
{
    i16 x, y;
    Client *c;
    Monitor *m;
    if(win == _wm->root && getrootptr(&x, &y)) return recttomon(x, y, 1, 1);
    for (m = _wm->mons; m; m = m->next)
        if (win == m->barwin) return m;
    if ((c = wintoclient(win))) return c->mon;
    return _wm->selmon;
}

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(err)
    {   
        DEBUG("%s %s\n", XCBErrorCodeText(err->error_code), XCBErrorMajorCodeText(err->major_code));
        DEBUG("error_code: [%d], major_code: [%d], minor_code: [%d]\n"
              "sequence: [%d], response_type: [%d], resource_id: [%d]\n"
              "full_sequence: [%d]\n"
              ,
           err->error_code, err->major_code, err->minor_code, 
           err->sequence, err->response_type, err->resource_id, 
           err->full_sequence);
    }
}



int
main(int argc, char *argv[])
{
    argcvhandler(argc, argv);
    startup();
    setup();
#ifdef __OpenBSD__
        if (pledge("stdio rpath proc exec", NULL) == -1)
            die("pledge");
#endif /* __OpenBSD__ */
    scan();
    run();
    cleanup();
    XCBCloseDisplay(_wm->dpy);
    u8 restart = _wm->restart;
    free(_wm);
    if(restart)
    {   execvp(argv[0], argv);
    }
    return EXIT_SUCCESS;
}
