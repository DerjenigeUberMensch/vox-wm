
#include "keybind/keybind_defaults.h"
#include "VXExtDebug/vxextdebug.h"

#include "XCB-TRL/xcb_trl_types.h"
#include "actions.h"
#include "interactive.h"
#include "main.h"

extern WM _wm;

void 
KeybindUserStats(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    ActionUserStats();
}

void 
KeybindSpawnWindow(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;

    char **data = arg->datav[0];

    if(!data)
    {   
        DebugError("No data");
        return;
    }

    if(!data[0])
    {   
        DebugError("Invalid data");
        return;
    }

    ActionSpawnWindow(data[0], (void *)data);
}

void 
KeybindToggleBar(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    ActionToggleStatusBar(m);
UNLOCK:
    UNLOCK_WM();
}

void 
KeybindKillWindow(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    Desktop *desk = m->desksel;

    if(unlikely(!desk))
    {
        DebugError("No selected desktop");
        goto UNLOCK;
    }

    Client *c = desk->sel;

    if(c)
    {   ActionKillWindow(c);
    }
UNLOCK:
    UNLOCK_WM();
}

void 
KeybindTerminateWindow(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    Desktop *desk = m->desksel;

    if(unlikely(!desk))
    {
        DebugError("No selected desktop");
        goto UNLOCK;
    }

    Client *c = desk->sel;

    if(c)
    {   ActionTerminateWindow(c);
    }
UNLOCK:
    UNLOCK_WM();
}

void 
KeybindMaximizeWindow(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    Desktop *desk = m->desksel;

    if(unlikely(!desk))
    {
        DebugError("No selected desktop");
        goto UNLOCK;
    }

    Client *c = desk->sel;

    if(c)
    {   ActionMaximizeWindow(c);
    }
UNLOCK:
    UNLOCK_WM();
}

void 
KeybindQuit(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    ActionQuit();

    UNLOCK_WM();
}

void 
KeybindRestart(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    ActionRestart();

    UNLOCK_WM();
}

void 
KeybindRestartQ(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    ActionRestartQ();

    UNLOCK_WM();
}

void 
KeybindStickWindow(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    Desktop *desk = m->desksel;

    if(unlikely(!desk))
    {
        DebugError("No selected desktop");
        goto UNLOCK;
    }

    Client *c = desk->sel;

    if(c)
    {   ActionStickWindow(c);
    }

UNLOCK:
    UNLOCK_WM();
}

void 
KeybindSetWindowLayout(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;

    enum LayoutType layout = arg->data32i[0];

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    ActionSetWindowLayout(m, layout);

UNLOCK:
    UNLOCK_WM();
}

void 
KeybindToggleFullScreen(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    Desktop *desk = m->desksel;

    if(unlikely(!desk))
    {
        DebugError("No selected desktop");
        goto UNLOCK;
    }

    Client *c = desk->sel;

    if(c)
    {   ActionToggleFullscreen(c);
    }

UNLOCK:
    UNLOCK_WM();
}

void 
KeybindToggleDesktop(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;

    i32 desktop = arg->data32i[0];

    LOCK_WM();

    Monitor *m = _wm.selmon;

    if(unlikely(!m))
    {
        DebugError("No selected monitor");
        goto UNLOCK;
    }

    ActionToggleDesktop(m, desktop);

UNLOCK:
    UNLOCK_WM();
}

/* buttons */

void 
KeybindResizeWindow(const WMKeyBind *keybind, const Generic *arg)
{
    i32 alt_mode = arg->data32i[0];
    XCBWindow win = XCBNone;
    XCBButton xbutton = XCBButtonAny;
    int use_ev = 0;

    if(keybind->ev)
    {
        int cleanev = XCB_EVENT_RESPONSE_TYPE(keybind->ev);

        if(cleanev == XCBButtonPress)
        {
            XCBButtonPressEvent *bev = (XCBButtonPressEvent *)keybind->ev;

            if(unlikely(bev->event == XCBNone))
            {   
                DebugWarn("Malformed request");
                return;
            }

            if(IS_WM_WINDOW(bev->event))
            {
                if(bev->child == XCBNone || unlikely(IS_WM_WINDOW(bev->child)))
                {   return;
                }

                win = bev->child;
            }
            else
            {   win = bev->event;
            }

            xbutton = CLEANBUTTONMASK(bev->detail);

            use_ev = 1;
        }
    }

    LOCK_WM();

    (void)ASSERT(GArrayEnd((void *)&keybind->buttons) - GArrayStart((void *)&keybind->buttons) == 1);

    WMButton *button = GArrayAt((void *)&keybind->buttons, 0);

    if(!use_ev)
    {   
        Monitor *m = _wm.selmon;

        if(unlikely(!m))
        {
            DebugError("No selected monitor");
            goto UNLOCK;
        }

        Desktop *desk = m->desksel;

        if(unlikely(!desk))
        {
            DebugError("No selected desktop");
            goto UNLOCK;
        }

        Client *c = desk->sel;

        if(!c)
        {
            DebugWarn("No selected client");
            goto UNLOCK;
        }

        xbutton = button->button;
        win = c->win;
    }

    ResizeWindow(win, xbutton, alt_mode);
UNLOCK:
    UNLOCK_WM();
}

void 
KeybindDragWindow(const WMKeyBind *keybind, const Generic *arg)
{
    (void)keybind;
    (void)arg;

    XCBWindow win = XCBNone;
    XCBButton xbutton = XCBButtonAny;
    int use_ev = 0;

    if(keybind->ev)
    {
        int cleanev = XCB_EVENT_RESPONSE_TYPE(keybind->ev);

        if(cleanev == XCBButtonPress)
        {
            XCBButtonPressEvent *bev = (XCBButtonPressEvent *)keybind->ev;

            if(unlikely(bev->event == XCBNone))
            {   
                DebugWarn("Malformed request");
                return;
            }

            if(IS_WM_WINDOW(bev->event))
            {
                if(bev->child == XCBNone || unlikely(IS_WM_WINDOW(bev->child)))
                {   return;
                }

                win = bev->child;
            }
            else
            {   win = bev->event;
            }

            xbutton = CLEANBUTTONMASK(bev->detail);

            use_ev = 1;
        }
    }


    LOCK_WM();

    (void)ASSERT(GArrayEnd((void *)&keybind->buttons) - GArrayStart((void *)&keybind->buttons) == 1);

    WMButton *button = GArrayAt((void *)&keybind->buttons, 0);

    if(!use_ev)
    {   
        Monitor *m = _wm.selmon;

        if(unlikely(!m))
        {
            DebugError("No selected monitor");
            goto UNLOCK;
        }

        Desktop *desk = m->desksel;

        if(unlikely(!desk))
        {
            DebugError("No selected desktop");
            goto UNLOCK;
        }

        Client *c = desk->sel;

        if(!c)
        {
            DebugWarn("No selected client");
            goto UNLOCK;
        }

        xbutton = button->button;
        win = c->win;
    }

    DragWindow(win, xbutton);

UNLOCK:
    UNLOCK_WM();
}
