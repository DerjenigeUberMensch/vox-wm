
#ifndef CONFIG_H_
#define CONFIG_H_

#include "dwm.h"
#include <X11/keysym.h>


static const Layout layouts[] = 
{
	/* symbol     arrange function */
    {"NULL",      NULL}
};

/* key definitions */
#define ALT         XCB_MOD_MASK_1
#define SUPER       XCB_MOD_MASK_4
#define CTRL        XCB_MOD_MASK_CONTROL
#define SHIFT       XCB_MOD_MASK_SHIFT
#define CAPSLOCK    XCB_MOD_MASK_LOCK
#define LMB         XCB_BUTTON_MASK_1
#define MMB         XCB_BUTTON_MASK_2
#define RMB         XCB_BUTTON_MASK_3
#define BUTTON4     XCB_BUTTON_MASK_4
#define BUTTON5     XCB_BUTTON_MASK_5

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

static const Key keys[] =
{
    /* KeyType                  modifier                 key        function        argument */
    { XCB_KEY_PRESS,            0|SHIFT,                      XK_a,      UserStats,              {0}},
    { 0, XK_A, 0, 0},
    { 0, XK_A, 0, 0},
    { 0, XK_A, 0, 0},
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = 
{
    {0,0,0,0}
};


#endif
