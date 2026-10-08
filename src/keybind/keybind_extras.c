#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <xcb/xcb.h>

#include "GArray/garray.h"
#include "keybind/keybind_definitions.h"
#include "keybind/keybinds.h"

extern const struct KeyCodeEntry KEYBIND_MOD_TABLE[];
extern const struct KeyCodeEntry KEYBIND_KEYCODE_TABLE[];
extern const size_t KEYBIND_MOD_TABLE_LENGTH;
extern const size_t KEYBIND_KEYCODE_TABLE_LENGTH;
extern WM _wm;
extern GArray keybind_keybinds;

static uint16_t WM_KEYBIND_FIND_MODIFIER_MASK_X11(const char *mod_str)
{
    int i;

    for(i = 0; i < KEYBIND_MOD_TABLE_LENGTH; ++i)
    {
        if(strcasecmp(mod_str, KEYBIND_MOD_TABLE[i].name) == 0)
        {   return (uint16_t)KEYBIND_MOD_TABLE[i].keycode;
        }
    }

    return XCB_NO_SYMBOL;
}

static XCBKeysym WM_KEYBIND_FIND_KEYSYM_X11(const char *keysym_str)
{
    int i;

    for(i = 0; i < KEYBIND_KEYCODE_TABLE_LENGTH; ++i)
    {
        if(strcasecmp(keysym_str, KEYBIND_KEYCODE_TABLE[i].name) == 0)
        {   return (XCBKeysym)KEYBIND_KEYCODE_TABLE[i].keycode;
        }
    }

    return XCB_NO_SYMBOL;
}

uint16_t 
WMKeybindModifierFromString(const char *mod_str)
{
    uint16_t mod = WM_KEYBIND_FIND_MODIFIER_MASK_X11(mod_str);

    if(mod)
    {   return mod;
    }

    return XCB_NO_SYMBOL;
}

XCBKeysym 
WMKeybindKeysymFromString(const char *keysym_str)
{
    XCBKeysym sym = WM_KEYBIND_FIND_KEYSYM_X11(keysym_str);

    /* prefer our keysyms since they are parsed without caps */
    if(sym != XCB_NO_SYMBOL)
    {   return sym;
    }
 
    sym = XStringToKeysym(keysym_str);

    KeySym xlib_sym = sym;
    KeySym xlib_lower = NoSymbol;
    KeySym xlib_upper = NoSymbol;

    /* our parser doesnt like uppercase */
    XConvertCase(xlib_sym, &xlib_lower, &xlib_upper);

    return (XCBKeysym)xlib_lower;
}

/*( TODO: I dont even know how this works even though I wrote it )*/
bool
WMKeybindHandler(u16 mod, XCBKeyCode code, enum WMKeyType type, bool is_button)
{
    if(!ASSERT(type == WM_INPUT_PRESS || type == WM_INPUT_RELEASE))
    {   
        DebugError("Invalid key type: %d", type);
        return false;
    }

    mod = CLEANMASK(mod);

    /* ONLY use lowercase cause we dont know how to handle anything else */
    /* Only use upercase cause we dont know how to handle anything else
     * sym = XCBKeySymbolsGetKeySym(_wm.syms,  keydetail, 0);
     */
    /* This Could work MAYBE allowing for upercase and lowercase Keybinds However that would complicate things due to our ability to mask Shift
     * sym = XCBKeySymbolsGetKeySym(_wm.syms, keydetail, cleanstate); 
     */
    XCBKeysym sym = XCB_NO_SYMBOL;

    if(!is_button)
    {
        sym = XCBKeySymbolsGetKeySym(_wm.syms, code, 0);
    }

    /* Debug("%d", sym); */

    garray_i i;
    garray_i j;

    bool ret = false;

    for(i = GArrayStart(&keybind_keybinds); i < GArrayEnd(&keybind_keybinds); ++i)
    {
        WMKeyBind *keybind = GArrayAt(&keybind_keybinds, i);

        if(!ASSERT(keybind))
        {   
            DebugError("Keybind is NULL");
            continue;
        }

        garray_i keylen    = GArrayEnd(&keybind->keysyms) - GArrayStart(&keybind->keysyms);
        garray_i buttonlen = GArrayEnd(&keybind->buttons) - GArrayStart(&keybind->buttons);

        garray_i inputlen = keylen + buttonlen;

        u16 mask = CLEANMASK(keybind->mod);

        i32 presscount = 0;
        bool inputfound = false;
        bool keybindRan = false;

        for(j = GArrayStart(&keybind->keysyms); j < GArrayEnd(&keybind->keysyms); ++j)
        {
            WMKey *key = GArrayAt(&keybind->keysyms, j);

            if(!ASSERT(key))
            {   
                DebugError("Key is NULL");
                continue;
            }

            if(!is_button && key->sym == sym)
            {   
                inputfound= true;
                key->pressed = (type == WM_INPUT_PRESS);
            }

            presscount += key->pressed;
        }

        for(j = GArrayStart(&keybind->buttons); j < GArrayEnd(&keybind->buttons); ++j)
        {
            WMButton *button = GArrayAt(&keybind->buttons, j);

            if(!ASSERT(button))
            {   
                DebugError("Button is NULL");
                continue;
            }

            if(is_button && button->button == code)
            {   
                inputfound= true;
                button->pressed = (type == WM_INPUT_PRESS);
            }

            presscount += button->pressed;
        }

        if(!inputfound || mask != mod)
        {   continue;
        }

        if(keybind->type != type)
        {   continue;
        }

        switch(type)
        {
            case WM_INPUT_PRESS:
            {
                /* all keys have been pressed execute them */
                if(presscount == inputlen)
                {   keybindRan = true;
                }

                break;
            }
            case WM_INPUT_RELEASE:
            {
                /* A key has been released from the keybind execute it */
                if(presscount == inputlen - 1)
                {   keybindRan = true;
                }

                break;
            }
        }

        if(keybindRan)
        {
            if(!keybind->func)
            {   
                if(GArrayEnd(&keybind->buttons) - GArrayStart(&keybind->buttons) > 0)
                {   /* ignore.... */
                }
                else
                {   DebugError("Callback is NULL");
                }

            }
            else
            {   
                keybind->func(keybind, &keybind->arg);
                ret = true;
            }
        }
    }

    return ret;
}


