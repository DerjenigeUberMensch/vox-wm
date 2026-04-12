#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "wmlua/input.h"
#include "main.h"
#include "keybinds.h"


enum
WMModMasks
{
    WM_ALT,
    WM_NUMLOCK,
    WM_SUPER,
    WM_CTRL,
    WM_SHIFT,
};

enum
WMButton
{
    WM_LMB,
    WM_MMB,
    WM_RMB,
    WM_SCROLL_UP,
    WM_SCROLL_DOWN,
    WM_SCROLL_LEFT,
    WM_SCROLL_RIGHT,
};

enum
WMKeysyms
{
    WM_ESCAPE,
    WM_RETURN,
    WM_SPACE,
    WM_BACKSPACE,
    WM_DELETE,
    WM_HOME,
    WM_END,
    WM_PAGE_UP,
    WM_PAGE_DOWN,

    WM_UP,
    WM_DOWN,
    WM_RIGHT,
    WM_LEFT,

    WM_TAB,
    WM_F1,
    WM_F2,
    WM_F3,
    WM_F4,
    WM_F5,
    WM_F6,
    WM_F7,
    WM_F8,
    WM_F9,
};

#define ALT         XCBMod1Mask
#define NUMLOCK     XCBMod2Mask
#define SUPER       XCBMod4Mask         /* "Command Key" "Windows Key" */
#define CTRL        XCBControlMask
#define SHIFT       XCBShiftMask
#define CAPSLOCK    XCBLockMask
#define ESCAPE      XK_Escape
#define TAB         XK_Tab
#define LMB         XCBButton1
#define MMB         XCBButton2
#define RMB         XCBButton3
#define ScrollUp    XCBButton4
#define ScrollDown  XCBButton5

/* taken from i3 */
#define ScrollLeft  6
#define ScrollRight 7


#define ENTER       XK_Return
#define RETURN      XK_Return
/* F1 - F12 */
#define XF1         XK_F1
#define XF2         XK_F2
#define XF3         XK_F3
#define XF4         XK_F4
#define XF5         XK_F5
#define XF6         XK_F6
#define XF7         XK_F7
#define XF8         XK_F8
#define XF9         XK_F9
#define XF10        XK_F10
#define XF11        XK_F11
#define XF12        XK_F12


/* XF86 */
#define XAudioMute                  XF86XK_AudioMute
#define XAudioLowerVolume           XF86XK_AudioLowerVolume
#define XAudioRaiseVolume           XF86XK_AudioRaiseVolume
#define XMonLowerBrightness         XF86XK_MonBrightnessDown
#define XMonRaiseBrightness         XF86XK_MonBrightnessUp
#define XAudioPlay                  XF86XK_AudioPlay
#define XAudioPause                 XF86XK_AudioPause
#define XAudioNext                  XF86XK_AudioNext
#define XAudioPrev                  XF86XK_AudioPrev

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }



typedef struct KeyCodeEntry KeyCodeEntry;

struct
KeyCodeEntry
{
    const char *name;
    u16 keycode;
};

static const KeyCodeEntry keycode_table[] =
{ 
    { "ctrl", CTRL },
    { "alt", ALT },
    { "shift", SHIFT },
    { "tab", TAB },
    { "escape", ESCAPE },

    { "lmb", LMB },

    { "mmb", MMB },

    { "rmb", RMB },

    { "scrollup", ScrollUp },
    { "scrolldown", ScrollDown },
    { "scrollleft", ScrollLeft },
    { "scrollright", ScrollRight},

    { "capslock", CAPSLOCK },
    { "caplock", CAPSLOCK },

    { "enter", ENTER },
    { "return", ENTER },

    { "f1", XF1 },
    { "f2", XF2 },
    { "f3", XF3 },
    { "f4", XF4 },
    { "f5", XF5 },
    { "f6", XF6 },
    { "f7", XF7 },
    { "f8", XF8 },
    { "f9", XF9 },
    { "f10", XF10 },
    { "f11", XF11 },
    { "f12", XF12 },

    { "", },

    { "super", SUPER },
    { "windowkey", SUPER },
    { "command", SUPER },

};


extern WM _wm;

int strcmp_lower(const char *a, const char *b)
{
    char ca;
    char cb;

    while (*a && *b) 
    {
        ca = tolower((unsigned char)*a);
        cb = tolower((unsigned char)*b);

        if (ca != cb) 
        {   return ca - cb;
        }

        ++a;
        ++b;
    }

    return *a - *b;
}

int l_input_bind(lua_State *l)
{
    const char *combo = lua_tostring(l, 1);

    if(!combo || combo[0] == '\0')
    {   return luaL_error(l, "invalid bind");
    }

    enum { BUFF_SIZE = 1024 };

    char buff[BUFF_SIZE];

    memset(buff, 0, BUFF_SIZE);

    strncpy(buff, combo, (BUFF_SIZE - 1) * sizeof(char));

    char *saveptr;

    u32 mask;
    XCBKeysym keycode;

    do
    {
        char *token = strtok_r(buff, "+", &saveptr);

        if(!token)
        {   break;
        }

        if(strcmp_lower(token, "ctrl"))
        {
        }


    } while(1);
}
