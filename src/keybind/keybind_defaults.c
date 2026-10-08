
#include "keybind/keybind_defaults.h"
#include "VXExtDebug/vxextdebug.h"

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

    (void)ASSERT(GArrayEnd((void *)&keybind->buttons) - GArrayStart((void *)&keybind->buttons) == 1);

    if(c)
    {   
        WMButton *button = GArrayAt((void *)&keybind->buttons, 0);
        XCBButton xbutton = XCBButtonAny;

        if(button)
        {   xbutton = button->button;
        }
        else
        {   DebugError("Button is NULL");
        }

        ResizeWindow(c->win, xbutton, alt_mode);
    }

UNLOCK:
    UNLOCK_WM();
}

void 
KeybindDragWindow(const WMKeyBind *keybind, const Generic *arg)
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

    (void)ASSERT(GArrayEnd((void *)&keybind->buttons) - GArrayStart((void *)&keybind->buttons) == 1);

    if(c)
    {   
        WMButton *button = GArrayAt((void *)&keybind->buttons, 0);
        XCBButton xbutton = XCBNone;

        if(button)
        {   xbutton = button->button;
        }
        else
        {   DebugError("Button is NULL");
        }

        DragWindow(c->win, xbutton);
    }

UNLOCK:
    UNLOCK_WM();
}
