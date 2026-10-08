#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xproto.h>

#include "VXExtDebug/vxextdebug.h"
#include "XCB-TRL/xcb_trl.h"
#include "client.h"
#include "decorations.h"
#include "keybind/keybind_defaults.h"
#include "keybind/keybind_definitions.h"
#include "main.h"
#include "safebool.h"
#include "util.h"
#include "GArray/garray.h"
#include "XCB-TRL/xcb_keysym.h"
#include "XCB-TRL/xcb_trl_types.h"

#include "keybind/keybinds.h"
#include "keybind/keybind_extras.h"
#include "keybind/keybind_util.h"

#include "legacy/keybinds.h"

extern WM _wm;

extern const struct KeyCodeEntry KEYBIND_MOD_TABLE[];
extern const struct KeyCodeEntry KEYBIND_KEYCODE_TABLE[];
extern const size_t KEYBIND_MOD_TABLE_LENGTH;
extern const size_t KEYBIND_KEYCODE_TABLE_LENGTH;

GArray keybind_keybinds = GARRAY_STATIC_INITIALIZER(sizeof(WMKeyBind));

int
WMKeybindInit(void)
{   
    int status = GArrayCreateFilled(&keybind_keybinds, sizeof(WMKeyBind), 0);

    if(status != EXIT_SUCCESS)
    {   return EXIT_FAILURE;
    }

    /* intialize the default keybinds 
     * These have been set since v1.0.0 and are considered the default keybinds for our wm.
     * Unforunately some keybinds werent very well thought out long term but due to backwards compatibility we will keep them as is. (cough cough SUPER+E)
     */

    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_N }, 1, NULL, 0, KeybindUserStats, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_D }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_dmenucmd }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_RETURN }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_termcmd }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_E }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_filemanager }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_B }, 1, NULL, 0, KeybindToggleBar, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER|WM_SHIFT, (XCBKeysym[]){ WM_Q }, 1, NULL, 0, KeybindKillWindow, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_CTRL|WM_ALT, (XCBKeysym[]){ WM_Q }, 1, NULL, 0, KeybindTerminateWindow, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_W }, 1, NULL, 0, KeybindMaximizeWindow, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER|WM_SHIFT, (XCBKeysym[]){ WM_P }, 1, NULL, 0, KeybindQuit, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER|WM_CTRL, (XCBKeysym[]){ WM_P }, 1, NULL, 0, KeybindRestartQ, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_CTRL|WM_ALT, (XCBKeysym[]){ WM_P }, 1, NULL, 0, KeybindRestart, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER|WM_CTRL, (XCBKeysym[]){ WM_Z }, 1, NULL, 0, KeybindStickWindow, (Generic){0}, WM_INPUT_PRESS);

    /* layouts */
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_Z }, 1, NULL, 0, KeybindSetWindowLayout, (Generic){ .data32i[0]= Tiled }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_X }, 1, NULL, 0, KeybindSetWindowLayout, (Generic){ .data32i[0]= Floating }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_C }, 1, NULL, 0, KeybindSetWindowLayout, (Generic){ .data32i[0]= Monocle }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_G }, 1, NULL, 0, KeybindSetWindowLayout, (Generic){ .data32i[0]= Grid }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_F11 }, 1, NULL, 0, KeybindToggleFullScreen, (Generic){0}, WM_INPUT_PRESS);

    /* desktop switching */
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_1 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 0 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_2 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 1 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_3 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 2 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_4 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 3 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_5 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 4 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_6 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 5 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_7 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 6 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_8 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 7 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_9 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 8 }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, (XCBKeysym[]){ WM_0 }, 1, NULL, 0, KeybindToggleDesktop, (Generic){ .data32i[0] = 9 }, WM_INPUT_PRESS);

    /* multimedia */
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_MUTE }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_mute_vol }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_VOL_DOWN }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_down_vol }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_VOL_UP }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_up_vol }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_MON_BRIGHTNESS_DOWN }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_dimmer }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_MON_BRIGHTNESS_UP }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_brighter }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_PLAY }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_pause_vol }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_PAUSE }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_pause_vol }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_NEXT }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_next_vol }, WM_INPUT_PRESS);
    WMKeybindAdd(0, (XCBKeysym[]){ WM_AUDIO_PREV }, 1, NULL, 0, KeybindSpawnWindow, (Generic){ .datav[0] = legacy_prev_vol }, WM_INPUT_PRESS);

    WMKeybindAdd(WM_SUPER, NULL, 0, (XCBButton[]){ WM_RMB }, 1, KeybindResizeWindow, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, NULL, 0, (XCBButton[]){ WM_RMB }, 1, NULL, (Generic){0}, WM_INPUT_RELEASE);

    /* debug */
    WMKeybindAdd(WM_SUPER|WM_ALT, NULL, 0, (XCBButton[]){ WM_RMB }, 1, KeybindResizeWindow, (Generic){ .data32i[0] = 1, }, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER|WM_ALT, NULL, 0, (XCBButton[]){ WM_RMB }, 1, NULL, (Generic){0}, WM_INPUT_RELEASE);

    WMKeybindAdd(WM_SUPER, NULL, 0, (XCBButton[]){ WM_LMB }, 1, KeybindDragWindow, (Generic){0}, WM_INPUT_PRESS);
    WMKeybindAdd(WM_SUPER, NULL, 0, (XCBButton[]){ WM_LMB }, 1, NULL, (Generic){0}, WM_INPUT_RELEASE);

    return EXIT_SUCCESS;
}

int
WMKeybindAdd(uint16_t xcb_modmask_modifiers, XCBKeysym keysyms[], size_t num_keysyms, XCBButton buttons[], size_t num_buttons, void (*func)(const WMKeyBind *self, const Generic *arg), Generic arg, enum WMKeyType type)
{
    enum { ERROR = 1, ALREADY_EXISTS = -1, SUCCESS = 0 };

    WMKeyBind k = 
    {
        .mod = xcb_modmask_modifiers,
        .type = type,
        .arg = arg,
        .func = func,
    };

    /* lock? */
    int status = GArrayCreateFilled(&k.keysyms, sizeof(WMKey), num_keysyms);

    if(status != EXIT_SUCCESS)
    {   return ERROR;
    }

    status = GArrayCreateFilled(&k.buttons, sizeof(WMButton), num_buttons);

    if(status != EXIT_SUCCESS)
    {   
        GArrayWipe(&k.keysyms);
        return ERROR;
    }

    size_t i;
    
    for(i = 0; i < num_keysyms; ++i)
    {   
        WMKey key = 
        {
            .sym = keysyms[i],
            .pressed = 0,
        };

        status = GArrayPushBack(&k.keysyms, &key);

        if(status != EXIT_SUCCESS)
        {   goto DESTROY;
        }
    }

    for(i = 0; i < num_buttons; ++i)
    {   
        WMButton button = 
        {
            .button = buttons[i],
            .pressed = 0,
        };

        status = GArrayPushBack(&k.buttons, &button);

        if(status != EXIT_SUCCESS)
        {   goto DESTROY;
        }
    }

    garray_i index = 0;

    status = KEYBIND_FIND_KEY(&k, &index);

    /* key already exists? */
    if(status == EXIT_SUCCESS)
    {
        DebugWarn("Keybind already exists at index %zu\n", index);
        GArrayWipe(&k.keysyms);
        GArrayWipe(&k.buttons);
        return ALREADY_EXISTS;
    }

    status = GArrayPushBack(&keybind_keybinds, &k);

    if(status != EXIT_SUCCESS)
    {   goto DESTROY;
    }

    return SUCCESS;
DESTROY:
    GArrayWipe(&k.keysyms);
    GArrayWipe(&k.buttons);

    return ERROR;
}

int
WMKeybindRemove(uint16_t modifier_mask_x11, XCBKeysym keysyms[], size_t num_keysyms, XCBButton buttons[], size_t num_buttons)
{
    WMKeyBind k = 
    {
        .mod = modifier_mask_x11,
        .type = WM_INPUT_PRESS,
        .keysyms = GARRAY_STATIC_INITIALIZER(sizeof(WMKey)),
        .buttons = GARRAY_STATIC_INITIALIZER(sizeof(WMButton)),
    };

    /* TODO: Make this not be this inefficient */
    garray_i i;

    for(i = 0; i < num_keysyms; ++i)
    {   
        WMKey key = 
        {
            .sym = keysyms[i],
            .pressed = 0,
        };

        int status = GArrayPushBack(&k.keysyms, &key);

        if(status != EXIT_SUCCESS)
        {   
            GArrayWipe(&k.keysyms);
            return EXIT_FAILURE;
        }
    }

    for(i = 0; i < num_buttons; ++i)
    {   
        WMButton button = 
        {
            .button = buttons[i],
            .pressed = 0,
        };

        int status = GArrayPushBack(&k.buttons, &button);

        if(status != EXIT_SUCCESS)
        {   
            GArrayWipe(&k.keysyms);
            return EXIT_FAILURE;
        }
    }

    int status;
    garray_i index = 0;

    status = KEYBIND_FIND_KEY(&k, &index);

    GArrayWipe(&k.keysyms);
    GArrayWipe(&k.buttons);

    /* key already exists? */
    if(status == EXIT_SUCCESS)
    {
        GArrayDelete(&keybind_keybinds, index);
    }
    else
    {
        DebugWarn("Keybind not found");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
WMKeybindRemoveAll(void)
{
    garray_i i;
    
    for(i = GArrayStart(&keybind_keybinds); i < GArrayEnd(&keybind_keybinds); ++i)
    {
        WMKeyBind *key = GArrayAt(&keybind_keybinds, i);
        GArrayWipe(&key->keysyms);
        GArrayWipe(&key->buttons);
    }

    GArrayResize(&keybind_keybinds, 0);
}

void
WMKeybindDestroy(void)
{
    garray_i i;

    for(i = GArrayStart(&keybind_keybinds); i < GArrayEnd(&keybind_keybinds); ++i)
    {
        WMKeyBind *key = GArrayAt(&keybind_keybinds, i);

        if(!ASSERT(key))
        {   
            DebugError("Key is NULL");
            continue;
        }

        GArrayWipe(&key->keysyms);
        GArrayWipe(&key->buttons);
    }

    GArrayWipe(&keybind_keybinds);

}

void
WMKeybindGrabKeys(void)
{
    garray_i i, j, k;

    XCBUngrabKey(_wm.dpy, XCB_GRAB_ANY, XCB_MOD_MASK_ANY, _wm.root);

    for(i = GArrayStart(&keybind_keybinds); i < GArrayEnd(&keybind_keybinds); ++i)
    {
        WMKeyBind *key = GArrayAt(&keybind_keybinds, i);

        if(!ASSERT(key))
        {   
            DebugError("key is NULL\n");
            continue;
        }

        /* managed by buttons */
        if(GArrayEnd(&key->buttons) - GArrayStart(&key->buttons) != 0)
        {   
            /* TODO */
            continue;
        }
        
        XCBKeyCode **codes = WM_KEYBIND_GENERATE_KEYCODES_X11(&key->keysyms);

        if(!codes)
        {   
            DebugError("code is NULL\n");
            continue;
        }

        for(j = GArrayStart(&key->keysyms); j < GArrayEnd(&key->keysyms); ++j)
        {
            XCBKeysym *sym = GArrayAt(&key->keysyms, j);

            if(!ASSERT(sym))
            {   
                DebugError("Keysym is NULL");
                continue;
            }

            if(!codes[j])
            {   
                DebugWarn("No keycode found for keysym %u\n", *sym);
                continue;
            }

            for(k = 0; codes[j][k] != XCB_NO_SYMBOL; ++k)
            {
                if(*sym == XCBKeySymbolsGetKeySym(_wm.syms, codes[j][k], 0))
                {   WM_KEYBIND_GRAB_KEYCODE_X11(codes[j][k], key->mod);
                }
            }

            free(codes[j]);
        }

        free(codes);
    }
}

static void
__ungrabbuttons(XCBWindow win, bool neverholdfocus, bool focused)
{
    garray_i i, j;
    /* numlock is int */
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK };
    /* somewhat taken from i3 */
    /* Always grab these to allow for replay pointer when focusing by mouse click */
    XCBButton gbuttons[3] = { WM_LMB, WM_MMB, WM_RMB };

    /* ungrab any previously grabbed buttons that are ours */
    for(i = 0; i < LENGTH(modifiers); ++i)
    {
        /* direct win grabs */
        if(!neverholdfocus)
        {
            for(j = 0; j < LENGTH(gbuttons); ++j)
            {   XCBUngrabButton(_wm.dpy, gbuttons[j], modifiers[i], win);
            }
        }

        for(j = GArrayStart(&keybind_keybinds); j < GArrayEnd(&keybind_keybinds); ++j)
        {
            WMKeyBind *key = GArrayAt(&keybind_keybinds, j);

            if(unlikely(!key))
            {
                DebugError("Key is NULL\n");
                continue;
            }

            garray_i k;

            for(k = GArrayStart(&key->buttons); k < GArrayEnd(&key->buttons); ++k)
            {
                WMButton *button = GArrayAt(&key->buttons, k);

                if(unlikely(!button))
                {
                    DebugError("Button is NULL\n");
                    continue;
                }

                XCBUngrabButton(_wm.dpy, button->button, modifiers[i], win);
            }
        }
    }
}


static void
__grabbuttons(XCBWindow win, bool neverholdfocus, bool focused)
{
    garray_i i, j;
    /* numlock is int */
    int modifiers[4] = { 0, XCB_MOD_MASK_LOCK, _wm.numlockmask, _wm.numlockmask|XCB_MOD_MASK_LOCK };
    /* somewhat taken from i3 */
    /* Always grab these to allow for replay pointer when focusing by mouse click */
    XCBButton gbuttons[3] = { WM_LMB, WM_MMB, WM_RMB };

    if (!focused)
    {
        /* grab focus buttons */
        if(!neverholdfocus)
        {
            for (i = 0; i < LENGTH(gbuttons); ++i)
            {
                for (j = 0; j < LENGTH(modifiers); ++j)
                {   
                    /* XCB_GRAB_MODE_SYNC for the keyboard beucase when tested there is one very tiny race (tested on about 255 windows quiting them all quickly)
                     * Where the focus is lost to another client before we process the button press. 
                     * Making it so more than 1 client believes it has focus.
                     * This is mostly a visual glitch but happens ocassioanly when using X
                     * So no reason to change it ASYNC since we arent managing hundreds of windows.
                     */
                    XCBGrabButton(_wm.dpy, gbuttons[i], modifiers[j], win, False, BUTTONMASK, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_SYNC, XCB_NONE, XCB_NONE);
                }
            }
        }
    }

    for(i = GArrayStart(&keybind_keybinds); i < GArrayEnd(&keybind_keybinds); ++i)
    {
        WMKeyBind *key = GArrayAt(&keybind_keybinds, i);

        if(unlikely(!key))
        {
            DebugError("Key is NULL\n");
            continue;
        }

        garray_i k;

        for(j = GArrayStart(&key->buttons); j < GArrayEnd(&key->buttons); ++j)
        {
            WMButton *button = GArrayAt(&key->buttons, j);

            if(unlikely(!button))
            {
                DebugError("Button is NULL\n");
                continue;
            }

            for(k = 0; k < LENGTH(modifiers); ++k)
            {
                XCBGrabButton(_wm.dpy, button->button, 
                            key->mod | modifiers[k], 
                            win, False, 
                            BUTTONMASK, 
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC, 
                            XCBNone, XCBNone
                );
            }
        }
    }
}

void
WMKeybindGrabButtons(Client *c, int focused)
{
    xcb_grab_server(_wm.dpy);

    __ungrabbuttons(c->decor->win, NEVERHOLDFOCUS(c), focused);
    __ungrabbuttons(c->win, NEVERHOLDFOCUS(c), focused);

    XCBWindow grab;

    if(ISDECORACTIVE(c))
    {   grab = c->decor->win;
    }
    else
    {   grab = c->win;
    }

    __grabbuttons(grab, NEVERHOLDFOCUS(c), focused);

    xcb_ungrab_server(_wm.dpy);
}
