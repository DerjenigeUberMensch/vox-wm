#ifndef KEYBINDS_H_
#define KEYBINDS_H_

#include "config.h"
#include "toggle.h"
#include "dwm.h"
#include <X11/keysym.h>

/* key definitions */
#define ALT         XCB_MOD_MASK_1
#define NUMLOCK     XCB_MOD_MASK_2
#define SUPER       XCB_MOD_MASK_4
#define CTRL        XCB_MOD_MASK_CONTROL
#define SHIFT       XCB_MOD_MASK_SHIFT
#define CAPSLOCK    XCB_MOD_MASK_LOCK
#define TAB         XK_Tab
#define LMB         XCB_BUTTON_MASK_1
#define MMB         XCB_BUTTON_MASK_2
#define RMB         XCB_BUTTON_MASK_3
#define BUTTON4     XCB_BUTTON_MASK_4
#define BUTTON5     XCB_BUTTON_MASK_5

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
    { XCB_KEY_PRESS,            SUPER,                  XK_Return,  SpawnWindow,    { .v = termcmd  }  },
    { XCB_KEY_PRESS,            SUPER,                  XK_d,       SpawnWindow,    { .v = dmenucmd }  },
    { XCB_KEY_PRESS,            SUPER|CTRL,             XK_p,       Restart,            {0} },
    { XCB_KEY_PRESS,            SUPER|SHIFT,            XK_p,       Quit,               {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = 
{
    {0,0,0,0}
};


#endif
