#include "keybind/keybinds.h"
#include <stdlib.h>

extern WM _wm;
extern GArray keybind_keybinds;

/* 
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FALURE on Failure.
 */
int
KEYBIND_FIND_KEY(WMKeyBind *find, garray_i *index_return)
{
    if(!ASSERT(find) || !ASSERT(index_return))
    {   return EXIT_FAILURE;
    }

    garray_i i;

    for(i = GArrayStart(&keybind_keybinds); i < GArrayEnd(&keybind_keybinds); ++i)
    {
        WMKeyBind *key = GArrayAt(&keybind_keybinds, i);

        if(unlikely(!key))
        {
            DebugWarn("Keybind at index %zu is NULL", i);
            continue;
        }

        if(key->mod != find->mod)
        {   continue;
        }

        if(key->type != find->type)
        {   continue;
        }

        u32 key_len = GArrayEnd(&key->keysyms) - GArrayStart(&key->keysyms);
        u32 find_len = GArrayEnd(&find->keysyms) - GArrayStart(&find->keysyms);

        if(key_len != find_len)
        {   continue;
        }

        u32 button_len = GArrayEnd(&key->buttons) - GArrayStart(&key->buttons);
        u32 find_button_len = GArrayEnd(&find->buttons) - GArrayStart(&find->buttons);

        if(button_len != find_button_len)
        {   continue;
        }

        garray_i j;
        garray_i k;

        garray_i matches = 0;

        for(j = GArrayStart(&key->keysyms); j < GArrayEnd(&key->keysyms); ++j)
        {
            for(k = GArrayStart(&find->keysyms); k < GArrayEnd(&find->keysyms); ++k)
            {
                WMKey *a = GArrayAt(&key->keysyms, j);
                WMKey *b = GArrayAt(&find->keysyms, k);

                if(a && b && a->sym == b->sym)
                {   ++matches;
                }
            }
        }

        /* If we didn't find a match for all keysyms, continue to the next keybind */
        if(matches != key_len && matches != find_len)
        {   continue;
        }

        for(j = GArrayStart(&key->buttons); j < GArrayEnd(&key->buttons); ++j)
        {
            for(k = GArrayStart(&find->buttons); k < GArrayEnd(&find->buttons); ++k)
            {
                WMButton *a = GArrayAt(&key->buttons, j);
                WMButton *b = GArrayAt(&find->buttons, k);

                if(a && b && a->button == b->button)
                {   ++matches;
                }
            }
        }

        /* If we didn't find a match for all buttons, continue to the next keybind */
        if(matches != button_len && matches != find_button_len)
        {   continue;
        }

        *index_return = i;
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

XCBKeyCode **
WM_KEYBIND_GENERATE_KEYCODES_X11(GArray *syms)
{
    XCBKeyCode **codes = malloc(sizeof(XCBKeyCode *) * (GArrayEnd(syms) - GArrayStart(syms)));

    if(!codes)
    {   return NULL;
    }

    size_t i;

    for(i = GArrayStart(syms); i < GArrayEnd(syms); ++i)
    {
        XCBKeysym *keysym = GArrayAt(syms, i);

        if(!ASSERT(keysym))
        {   
            DebugError("Keysym is NULL");
            continue;
        }

        XCBKeyCode *code = XCBKeySymbolsGetKeyCode(_wm.syms, *keysym);

        if(!code)
        {   DebugWarn("Failed to get keycode for keysym %u\n", *keysym);
        }

        codes[i] = code;
    }

    return codes;
}

void 
WM_KEYBIND_GRAB_KEYCODE_X11(XCBKeyCode code, u16 mod)
{
    enum 
    {
        IGNORE_NONE,
        IGNORE_CAPSLOCK,
        IGNORE_NUMLOCK,
        IGNORE_ALL,
        IGNORE_LAST,
    };

    /* treat empty, numlock state, capslock, all as same */
    const u32 ignoremodifiers[IGNORE_LAST] = 
    {
        [IGNORE_NONE] = 0,
        [IGNORE_CAPSLOCK] = XCB_MOD_MASK_LOCK,
        [IGNORE_NUMLOCK] = _wm.numlockmask,
        [IGNORE_ALL] = _wm.numlockmask|XCB_MOD_MASK_LOCK
    };

    int i;

    for(i = 0; i < LENGTH(ignoremodifiers); ++i)
    {
        u16 currentMod = mod;

        /* if its modmask any then appening ignore modifiers breaks X11, so dont... */
        if(currentMod != XCBModMaskAny)
        {   currentMod|= ignoremodifiers[i];
        }

        XCBGrabKey(_wm.dpy, 
            code, currentMod,
            _wm.root, true, 
            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
}