#ifndef KEYBINDS_H_
#define KEYBINDS_H_

#include "config.h"
#include "toggle.h"
#include "dwm.h"
#include <X11/keysym.h>
#include <X11/XF86keysym.h> 

/* key definitions */
#define ALT         XCBMod1Mask
#define NUMLOCK     XCBMod2Mask
#define SUPER       XCBMod4Mask
#define CTRL        XCBControlMask
#define SHIFT       XCBShiftMask
#define CAPSLOCK    XCBLockMask
#define TAB         XK_Tab
#define LMB         XCBButton1
#define MMB         XCBButton2
#define RMB         XCBButton3
#define BUTTON4     XCBButton4
#define BUTTON5     XCBButton5

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* Multimedia Keys (laptops and function keyboards) */
static const char *up_vol[]   = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+1%",   NULL };
static const char *down_vol[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-1%",   NULL };
static const char *mute_vol[] = { "pactl", "set-sink-mute",   "@DEFAULT_SINK@", "toggle", NULL };
static const char *pause_vol[]= { "playerctl", "play-pause"};
static const char *next_vol[] = { "playerctl", "next"};
static const char *prev_vol[] = { "playerctl", "previous"};
static const char *brighter[] = { "brightnessctl", "set", "1%+", NULL };
static const char *dimmer[]   = { "brightnessctl", "set", "1%-", NULL };

/* commands */
static char dmenumon[2] = "0"; 
static const char *dmenucmd[] = 
{ 
    "dmenu_run", "-m", dmenumon, "-fn", dmenufont, 
    "-nb", CFG_DMENU_COL_NORM_BACKGROUND, "-nf", CFG_DMENU_COL_NORM_FOREGROUND, 
    "-sb", CFG_DMENU_COL_SEL_BACKGROUND, "-sf", CFG_DMENU_COL_SEL_FOREGROUND, 
    CFG_DMENU_TOP_BAR ? NULL : "-b", CFG_DMENU_FAST_INPUT ? "-f" : NULL ,CFG_DMENU_CASE_SENSITIVE ? "-i" : NULL, NULL
}; /* flags -b == bottom bar; -f == getkeyboard input first then handle request; */

static const char *termcmd[]        = { "st", NULL };
static const char *filemanager[]    = {"thunar", NULL };
static const Key keys[] =
{
    /*Action            modifier                    key         function            argument */
    { XCBKeyPress,         SUPER,                   XK_n,       UserStats,          {0} },
    { XCBKeyPress,         SUPER,                   XK_d,       SpawnWindow,        {.v = dmenucmd } },
    { XCBKeyPress,         SUPER,                   XK_Return,  SpawnWindow,        {.v = termcmd } },
    { XCBKeyPress,         SUPER,                   XK_e,       SpawnWindow,        {.v = filemanager } },
    { XCBKeyPress,         SUPER,                   XK_b,       ToggleStatusBar,    {0} },
    { XCBKeyPress,         SUPER,                   XK_q,	    View,               {0} },
    { XCBKeyPress,         SUPER|SHIFT,             XK_q,       KillWindow,         {0} },
    { XCBKeyPress,         CTRL|ALT,                XK_q,	    TerminateWindow,    {0} },
    { XCBKeyPress,         SUPER,                   XK_w,       MaximizeWindow,     {0} },
    { XCBKeyRelease,       SUPER|SHIFT,             XK_p,       Quit,               {0} },
    { XCBKeyPress,         SUPER|CTRL,              XK_p,       Restart,            {0} },  /* UNSAFE sscanf() */
    { XCBKeyPress,         SUPER,                   XK_z,       SetWindowLayout,    {Tiled} },
    { XCBKeyPress,         SUPER,                   XK_x,       SetWindowLayout,    {Floating} },
    { XCBKeyPress,         SUPER,                   XK_c,       SetWindowLayout,    {Monocle} },
    { XCBKeyPress,         SUPER,                   XK_g,       SetWindowLayout,    {Grid} },
    { XCBKeyPress,         0,                       XK_F11,     ToggleFullscreen,   {0} },
    { XCBKeyPress,         ALT,                     TAB,        AltTab,	            {0} },

    { XCBKeyPress,         SUPER,                   XK_v,       SetBorderWidth,    { .i = +4 } },
    { XCBKeyPress,         SUPER,                   XK_f,       SetBorderWidth,    { .i = -4 } },
    /* multimedia keys */
    { XCBKeyPress,         0, XF86XK_AudioMute,                    SpawnWindow,        {.v = mute_vol } },
    { XCBKeyPress,         0, XF86XK_AudioLowerVolume,             SpawnWindow,        {.v = down_vol } },
    { XCBKeyPress,         0, XF86XK_AudioRaiseVolume,             SpawnWindow,        {.v = up_vol } },
    { XCBKeyPress,         0, XF86XK_MonBrightnessDown,            SpawnWindow,        {.v = dimmer } },
    { XCBKeyPress,         0, XF86XK_MonBrightnessUp,              SpawnWindow,        {.v = brighter } },
    { XCBKeyPress,         0, XF86XK_AudioPlay,                    SpawnWindow,        {.v = pause_vol } },
    { XCBKeyPress,         0, XF86XK_AudioPause,                   SpawnWindow,        {.v = pause_vol } },
    { XCBKeyPress,         0, XF86XK_AudioNext,                    SpawnWindow,        {.v = next_vol } },
    { XCBKeyPress,         0, XF86XK_AudioPrev,                    SpawnWindow,        {.v = prev_vol } },
};

static const Button buttons[] = 
{
    /* Type                 Button      Mask        function        arg */
    { XCB_BUTTON_PRESS,     RMB,        SUPER,      ResizeWindow,   {.i = RMB} },
    { XCB_BUTTON_PRESS,     LMB,        SUPER,      ResizeWindow,   {.i = LMB} },
    { XCB_BUTTON_PRESS,     MMB,        SUPER,      ResizeWindow,   {.i = MMB} },
};


#endif
