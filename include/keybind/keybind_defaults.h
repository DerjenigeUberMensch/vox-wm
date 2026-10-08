#ifndef __KEYBIND__DEFAULTS__H__
#define __KEYBIND__DEFAULTS__H__

#include "keybinds.h"

/* keysy */

void KeybindUserStats(const WMKeyBind *keybind, const Generic *arg);
void KeybindSpawnWindow(const WMKeyBind *keybind, const Generic *arg);
void KeybindToggleBar(const WMKeyBind *keybind, const Generic *arg);
void KeybindKillWindow(const WMKeyBind *keybind, const Generic *arg);
void KeybindTerminateWindow(const WMKeyBind *keybind, const Generic *arg);
void KeybindMaximizeWindow(const WMKeyBind *keybind, const Generic *arg);
void KeybindQuit(const WMKeyBind *keybind, const Generic *arg);
void KeybindRestart(const WMKeyBind *keybind, const Generic *arg);
void KeybindRestartQ(const WMKeyBind *keybind, const Generic *arg);
void KeybindStickWindow(const WMKeyBind *keybind, const Generic *arg);
void KeybindSetWindowLayout(const WMKeyBind *keybind, const Generic *arg);
void KeybindToggleFullScreen(const WMKeyBind *keybind, const Generic *arg);
void KeybindToggleDesktop(const WMKeyBind *keybind, const Generic *arg);

/* buttons */

void KeybindResizeWindow(const WMKeyBind *keybind, const Generic *arg);
void KeybindDragWindow(const WMKeyBind *keybind, const Generic *arg);





#endif