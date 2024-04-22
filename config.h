#ifndef CONFIG_H_
#define CONFIG_H_

#include "dwm.h"
#include <stdint.h>

/* See LICENSE file for copyright and license details.
 * 4 Tab spaces; No tab characters use spaces for tabs
 * Basic overview of dwm => https://ratfactor.com/dwm
 * For more information about xlib (X11)       visit https://x.org/releases/current/doc/libX11/libX11/libX11.html
 * For a quick peak at commonly used functions visit https://tronche.com/gui/x/xlib/
 * Cursors : https://tronche.com/gui/x/xlib/appendix/b/
 * XCursor:  https://man.archlinux.org/man/Xcursor.3
 * EWMH:     https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html
 * XEvent:   https://tronche.com/gui/x/xlib/events/structures.html
 * Document: https://www.x.org/releases/X11R7.5/doc/x11proto/proto.pdf
 */

/* FOR NON C USERS
 * This is where you can change the settings
 * You simply change to any of the things are there
 * so if its a number make it a number if its a number with .0 you can make any .0 number
 * However if you cant then dont
 * Secondly to see the changes happen you need to "recompile"
 * You can do this by making the changes you, saving and typing this in console
 * "sudo make install"
 * After that you simply Restart with the default keybinding below
 * Ctrl+WindowKey+p
 */

/* layout(s) */
enum LayoutType
{
    Tiled, Floating, Monocle, Grid
};

enum VarType
{
    INTEGER,
    FLOAT,
    DOUBLE,
    CHAR,
    STRING,
};

typedef struct CFGSave CFGSave;
typedef struct CFGSaveItem CFGSaveItem;

struct CFGSaveItem
{
    uint8_t _type;
    void *data;
    char *name;
    CFGSaveItem *next;
    CFGSaveItem *prev;
};

struct CFGSave
{
    char *file;
    CFGSaveItem *items;
    uint8_t redeable;
};

CFGSave *CFGCreateSave(const char *file_name, uint8_t human_redeable);
uint8_t CFGCreateVar(CFGSave *cfgsf, const char *var_name, uint8_t _type);
uint8_t CFGSaveVar(CFGSave *cfgsf, const char *save, void *data);

static const Layout layouts[4] =
{
    /* Name          symbol     arrange function */
    [Tiled]     = { "[T]",      tile            },
    [Floating]  = { "[F]",      floating        },
    [Monocle]   = { "[M]",      monocle         },
    [Grid]      = { "[G]",      grid            },
};


/* DEBUGGING
 * Stuff you need gdb xephyr
 * sudo pacman -S gdb xorg-server-xephyr
 *
 *
 * first make sure its compiled in DEBUG using config.mk
 *
 * run this command: Xephyr :1 -ac -resizeable -screen 680x480 &
 * set the display to the one you did for Xephyr in this case we did 1 so
 * run this command: export DISPLAY=:1
 * now you are mostly done
 * run this command: gdb dwm
 * you get menu
 * run this command: lay split
 * you get layout and stuff
 * 
 * now basic gdb stuff
 * break or b somefunction # this sets a break point for whatever function AKA stop the code from running till we say so
 * next or n # this moves to the next line of logic code (logic code is current code line)
 * step or s # this moves to the next line of code (code being actual code so functions no longer exist instead we just go there)
 * ctrl-l # this resets the window thing which can break sometimes (not sure why it hasnt been fixed but ok)
 * skip somefunction # this tries to skip a function if it can but ussualy is worthless (AKA I dont know how to use it)(skip being not show to you but it does run in the code)
 *
 * after your done
 * run this command: exit
 * you have succesfully used gdb wow
 * 
 */

#endif
