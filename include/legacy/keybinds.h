#ifndef KEYBINDS_H_
#define KEYBINDS_H_

#include <string.h>

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* Multimedia Keys (laptops and function keyboards) */
static const char *legacy_up_vol[]   = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+1%",   NULL };
static const char *legacy_down_vol[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-1%",   NULL };
static const char *legacy_mute_vol[] = { "pactl", "set-sink-mute",   "@DEFAULT_SINK@", "toggle", NULL };
static const char *legacy_pause_vol[]= { "playerctl", "play-pause"};
static const char *legacy_next_vol[] = { "playerctl", "next"};
static const char *legacy_prev_vol[] = { "playerctl", "previous"};
static const char *legacy_brighter[] = { "brightnessctl", "set", "1%+", NULL };
static const char *legacy_dimmer[]   = { "brightnessctl", "set", "1%-", NULL };

#define CFG_DMENU_TOP_BAR           0       /* 1 show dmenu bar on top; 0 for bottom bar                        */
#define CFG_DMENU_FAST_INPUT        0       /* 1 prioritize input over bar render; 0 to disable                 */
#define CFG_DMENU_CASE_SENSITIVE    1       /* 1 dmenu searches case senstivly; 0 to disable                    */
#define CFG_DMENU_COL_NORM_BACKGROUND       "#000000" /* dmenu background colour for NON selected items */
#define CFG_DMENU_COL_NORM_FOREGROUND       "#ffffff" /* dmenu text colour for NON selected items       */
#define CFG_DMENU_COL_SEL_BACKGROUND        "#000000" /* dmenu background colour for SELECTED items     */ 
#define CFG_DMENU_COL_SEL_FOREGROUND        "#ffffff" /* dmenu text colour for SELECTED items           */
/* commands */
static const char legacy_dmenufont[]   =   {"monospace:size=12"};
static const char *legacy_dmenucmd[] = 
{ 
    "dmenu_run", "-fn", legacy_dmenufont, 
    "-nb", CFG_DMENU_COL_NORM_BACKGROUND, "-nf", CFG_DMENU_COL_NORM_FOREGROUND, 
    "-sb", CFG_DMENU_COL_SEL_BACKGROUND, "-sf", CFG_DMENU_COL_SEL_FOREGROUND, 
    CFG_DMENU_TOP_BAR ? NULL : "-b", CFG_DMENU_FAST_INPUT ? "-f" : NULL ,CFG_DMENU_CASE_SENSITIVE ? "-i" : NULL, NULL
}; /* flags -b == bottom bar; -f == getkeyboard input first then handle request; */

static const char *legacy_termcmd[]        = { "st", NULL};
static const char *legacy_filemanager[2]    = {"thunar", NULL };

#endif