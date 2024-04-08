#ifndef KEYBINDS_H_
#define KEYBINDS_H_

#include "config.h"
#include "toggle.h"
#include "dwm.h"
#include <X11/keysym.h>

/* key definitions */
#define ALT         XCBMod1Mask
#define NUMLOCK     XCBMod2Mask
#define SUPER       XCBMod4Mask
#define CTRL        XCBControlMask
#define SHIFT       XCBShiftMask
#define CAPSLOCK    XCBLockMask
#define TAB         XK_Tab
#define LMB         XCBButton1Mask
#define MMB         XCBButton2Mask
#define RMB         XCBButton3Mask
#define BUTTON4     XCBButton4Mask
#define BUTTON5     XCBButton5Mask

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

static char dmenumon[2] = "0"; 
static const char *dmenucmd[] = 
{ 
    "dmenu_run", "-m", dmenumon, "-fn", dmenufont, 
    "-nb", CFG_DMENU_COL_NORM_BACKGROUND, "-nf", CFG_DMENU_COL_NORM_FOREGROUND, 
    "-sb", CFG_DMENU_COL_SEL_BACKGROUND, "-sf", CFG_DMENU_COL_SEL_FOREGROUND, 
    CFG_DMENU_TOP_BAR ? NULL : "-b", CFG_DMENU_FAST_INPUT ? "-f" : NULL ,CFG_DMENU_CASE_SENSITIVE ? "-i" : NULL, NULL
}; /* flags -b == bottom bar; -f == getkeyboard input first then handle request; */

static const char *termcmd[]        = { "st", NULL };
static const Key keys[] =
{
    /* KeyType                  modifier                 key        function        argument */
    { XCB_KEY_PRESS,            SUPER,                  XK_Return,  SpawnWindow,        { .v = termcmd  }  },
    { XCB_KEY_PRESS,            SUPER,                  XK_d,       SpawnWindow,        { .v = dmenucmd }  },
    { XCB_KEY_PRESS,            SUPER,                  XK_z,       SetWindowLayout,    { .ui = Tiled}  },
    { XCB_KEY_PRESS,            SUPER,                  XK_x,       SetWindowLayout,    { .ui = Floating }  },
    { XCB_KEY_PRESS,            SUPER,                  XK_c,       SetWindowLayout,    { .ui = Monocle}  },
    { XCB_KEY_PRESS,            SUPER,                  XK_g,       SetWindowLayout,    { .ui = Grid}  },

    { XCB_KEY_PRESS,            SUPER,                  XK_n,       UserStats,          { 0 }  },

    { XCB_KEY_PRESS,            CTRL|SUPER,             XK_p,       Restart,            { 0 }  },
    { XCB_KEY_RELEASE,          SHIFT|SUPER,            XK_p,       Quit,               { 0 }  }
};
/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = 
{
    { XCB_BUTTON_PRESS,     SUPER,      RMB,        ResizeWindow,   {.i = RMB} },
};


#endif
