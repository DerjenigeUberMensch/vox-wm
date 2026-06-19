#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "VXExtDebug/vxextdebug.h"
#include "XCB-TRL/xcb_trl.h"
#include "main.h"
#include "safebool.h"
#include "util.h"
#include "GArray/garray.h"
#include "XCB-TRL/xcb_keysym.h"
#include "XCB-TRL/xcb_trl_types.h"

#include "keybinds.h"

extern WM _wm;

GArray keybinds = GARRAY_STATIC_INITIALIZER(sizeof(Key));

typedef struct KeyCodeEntry KeyCodeEntry;

struct
KeyCodeEntry
{
    const char *name;
    u32 keycode;
};


static const KeyCodeEntry mods_table[] =
{
    { "mod1", WM_ALT },
    { "modmask1", WM_ALT },
    { "alt", WM_ALT },
    { "alternate", WM_ALT },
    { "alt", WM_ALT },
    { "option", WM_ALT },
    { "opt", WM_ALT },

    { "mod2", WM_NUMLOCK },
    { "modmask2", WM_NUMLOCK },
    { "num", WM_NUMLOCK },
    { "numlock", WM_NUMLOCK },
    { "num_lock", WM_NUMLOCK },
    { "numlk", WM_NUMLOCK },
    { "numlockkey", WM_NUMLOCK },
    { "numlock_key", WM_NUMLOCK },

    { "mod4", WM_SUPER },
    { "modmask4", WM_SUPER },
    { "super", WM_SUPER },
    { "win", WM_SUPER },
    { "window", WM_SUPER },
    { "windows", WM_SUPER },
    { "windowkey", WM_SUPER },
    { "windowskey", WM_SUPER },
    { "meta", WM_SUPER },
    { "cmd", WM_SUPER },
    { "command", WM_SUPER },
    { "logo", WM_SUPER },

    { "modcaps", WM_CAPSLOCK },
    { "lockmask", WM_CAPSLOCK },
    { "capslockmask", WM_CAPSLOCK },
    { "capslock", WM_CAPSLOCK },
    { "caps_lock", WM_CAPSLOCK },
    { "caplock", WM_CAPSLOCK },
    { "capslk", WM_CAPSLOCK },

    { "modcontrol", WM_CTRL },
    { "controlmask", WM_CTRL },
    { "ctrl", WM_CTRL },
    { "control", WM_CTRL },
    { "ctl", WM_CTRL },

    { "modshift", WM_SHIFT },
    { "shiftmask", WM_SHIFT },
    { "shift", WM_SHIFT },
};

static const KeyCodeEntry keycode_table[] =
{ 
    { "leftalt", WM_ALT_LEFT },
    { "altleft", WM_ALT_LEFT },
    { "alt_left", WM_ALT_LEFT },
    { "left_alt", WM_ALT_LEFT },
    { "lalt", WM_ALT_LEFT },
    { "altl", WM_ALT_LEFT },
    { "alt_l", WM_ALT_LEFT },
    { "l_alt", WM_ALT_LEFT },

    { "rightalt", WM_ALT_RIGHT },
    { "altright", WM_ALT_RIGHT },
    { "alt_right", WM_ALT_RIGHT },
    { "right_alt", WM_ALT_RIGHT },
    { "ralt", WM_ALT_RIGHT },
    { "altr", WM_ALT_RIGHT },
    { "alt_r", WM_ALT_RIGHT },
    { "r_alt", WM_ALT_RIGHT },

    { "leftshift", WM_SHIFT_LEFT },
    { "shiftleft", WM_SHIFT_LEFT },
    { "shift_left", WM_SHIFT_LEFT },
    { "left_shift", WM_SHIFT_LEFT },
    { "shiftl", WM_SHIFT_LEFT },
    { "lshift", WM_SHIFT_LEFT },
    { "shift_l", WM_SHIFT_LEFT },
    { "l_shift", WM_SHIFT_LEFT },

    { "rightshift", WM_SHIFT_RIGHT },
    { "shiftright", WM_SHIFT_RIGHT },
    { "shift_right", WM_SHIFT_RIGHT },
    { "right_shift", WM_SHIFT_RIGHT },
    { "shiftr", WM_SHIFT_RIGHT },
    { "rshift", WM_SHIFT_RIGHT },
    { "shift_r", WM_SHIFT_RIGHT },
    { "r_shift", WM_SHIFT_RIGHT },

    { "leftctrl", WM_CTRL_LEFT },
    { "leftcontrol", WM_CTRL_LEFT },
    { "ctrlleft", WM_CTRL_LEFT },
    { "ctrleft", WM_CTRL_LEFT },
    { "controlleft", WM_CTRL_LEFT },
    { "controleft", WM_CTRL_LEFT },
    { "left_ctrl", WM_CTRL_LEFT },
    { "left_control", WM_CTRL_LEFT },
    { "ctrl_left", WM_CTRL_LEFT },
    { "control_left", WM_CTRL_LEFT },
    { "lctrl", WM_CTRL_LEFT },
    { "lcontrol", WM_CTRL_LEFT },
    { "ctrll", WM_CTRL_LEFT },
    { "controll", WM_CTRL_LEFT },
    { "ctrl_l", WM_CTRL_LEFT },
    { "control_l", WM_CTRL_LEFT },
    { "l_ctrl", WM_CTRL_LEFT },
    { "l_control", WM_CTRL_LEFT },

    { "rightctrl", WM_CTRL_RIGHT },
    { "rightcontrol", WM_CTRL_RIGHT },
    { "ctrlright", WM_CTRL_RIGHT },
    { "ctrright", WM_CTRL_RIGHT },
    { "ctright", WM_CTRL_RIGHT },
    { "ctrl_right", WM_CTRL_RIGHT },
    { "control_right", WM_CTRL_RIGHT },
    { "right_ctrl", WM_CTRL_RIGHT },
    { "right_control", WM_CTRL_RIGHT },
    { "rctrl", WM_CTRL_RIGHT },
    { "rcontrol", WM_CTRL_RIGHT },
    { "ctrlr", WM_CTRL_RIGHT },
    { "controlr", WM_CTRL_RIGHT },
    { "ctrl_r", WM_CTRL_RIGHT },
    { "control_r", WM_CTRL_RIGHT },
    { "r_ctrl", WM_CTRL_RIGHT },
    { "r_control", WM_CTRL_RIGHT },

    { "tab", WM_TAB },
    { "tabkey", WM_TAB },
    { "tab_key", WM_TAB },

    { "escape", WM_ESCAPE },
    { "escape_key", WM_ESCAPE },
    { "escap", WM_ESCAPE },
    { "escpe", WM_ESCAPE },
    { "esc", WM_ESCAPE },

    { "lmb", WM_LMB },
    { "lmouse", WM_LMB },
    { "mousel", WM_LMB },
    { "leftmouse", WM_LMB },
    { "mouseleft", WM_LMB },
    { "left_mouse", WM_LMB },
    { "mouse_left", WM_LMB },
    { "mouse1", WM_LMB },
    { "button1", WM_LMB },
    { "btn1", WM_LMB },

    { "mmb", WM_MMB },
    { "mmouse", WM_MMB },
    { "mousem", WM_MMB },
    { "midlemouse", WM_MMB },
    { "middlemouse", WM_MMB },
    { "mousemiddle", WM_MMB },
    { "mousemidle", WM_MMB },
    { "middle_mouse", WM_MMB },
    { "midle_mouse", WM_MMB },
    { "mouse_middle", WM_MMB },
    { "mouse_midle", WM_MMB },
    { "mouse2", WM_MMB },
    { "button2", WM_MMB },
    { "btn2", WM_MMB },

    { "rmb", WM_RMB },
    { "rmouse", WM_RMB },
    { "mouser", WM_RMB },
    { "rightmouse", WM_RMB },
    { "mouseright", WM_RMB },
    { "mouse_right", WM_RMB },
    { "right_mouse", WM_RMB },
    { "mouse3", WM_RMB },
    { "button3", WM_RMB },
    { "btn3", WM_RMB },

    { "scrollup", WM_SCROLL_UP },
    { "scroll_up", WM_SCROLL_UP },
    { "wheel_up", WM_SCROLL_UP },
    { "mousewheelup", WM_SCROLL_UP },
    { "mwheelup", WM_SCROLL_UP },
    { "scrollupward", WM_SCROLL_UP },

    { "scrolldown", WM_SCROLL_DOWN },
    { "scroll_down", WM_SCROLL_DOWN },
    { "wheel_down", WM_SCROLL_DOWN },
    { "mousewheeldown", WM_SCROLL_DOWN },
    { "mwheeldown", WM_SCROLL_DOWN },
    { "scrolldownward", WM_SCROLL_DOWN },

    { "scrolllock", WM_SCROLL_LOCK },
    { "scroll_lock", WM_SCROLL_LOCK },
    { "scrolllk", WM_SCROLL_LOCK },
    { "scrolllockkey", WM_SCROLL_LOCK },
    { "scrolllock_key", WM_SCROLL_LOCK },

    { "backspace", WM_BACKSPACE },
    { "back_space", WM_BACKSPACE },
    { "back_spac", WM_BACKSPACE },

    { "delete", WM_DELETE },
    { "del", WM_DELETE },
    { "deletekey", WM_DELETE },
    { "delete_key", WM_DELETE },

    { "minus", WM_MINUS }, 
    { "minuskey", WM_MINUS }, 
    { "subtract", WM_MINUS }, 
    { "subtractkey", WM_MINUS }, 
    { "minus_key", WM_MINUS },

    { "plus", WM_PLUS }, 
    { "pluskey", WM_PLUS }, 
    { "add", WM_PLUS }, 
    { "addkey", WM_PLUS }, 
    { "plus_key", WM_PLUS },

    { "equal", WM_EQUALS }, 
    { "equalkey", WM_EQUALS }, 
    { "equals", WM_EQUALS }, 
    { "equalskey", WM_EQUALS }, 
    { "equals_key", WM_EQUALS },

    { "backquote", WM_BACKQUOTE },
    { "grave", WM_BACKQUOTE },
    { "graveaccent", WM_BACKQUOTE },
    { "back_quote", WM_BACKQUOTE },
    { "backquate", WM_BACKQUOTE },

    { "quote", WM_QUOTE },
    { "quotekey", WM_QUOTE },
    { "quote_key", WM_QUOTE },

    { "leftbracket", WM_LEFT_BRACKET },
    { "lbracket", WM_LEFT_BRACKET },
    { "left_bracket", WM_LEFT_BRACKET },
    { "leftbracketkey", WM_LEFT_BRACKET },
    { "leftbracket_key", WM_LEFT_BRACKET },

    { "rightbracket", WM_RIGHT_BRACKET },
    { "rbracket", WM_RIGHT_BRACKET },
    { "right_bracket", WM_RIGHT_BRACKET },
    { "rightbracketkey", WM_RIGHT_BRACKET },
    { "rightbracket_key", WM_RIGHT_BRACKET },

    { "semicolon", WM_SEMICOLON },
    { "semicolonkey", WM_SEMICOLON },
    { "semicolon_key", WM_SEMICOLON },

    { "apostrophe", WM_APOSTROPHE },
    { "apostroph", WM_APOSTROPHE },
    { "apostrophekey", WM_APOSTROPHE },
    { "apostrophe_key", WM_APOSTROPHE },

    { "comma", WM_COMMA },
    { "comma_key", WM_COMMA },

    { "period", WM_PERIOD },
    { "period_key", WM_PERIOD },

    { "slash", WM_SLASH },
    { "slash_key", WM_SLASH },

    { "backslash", WM_BACKSLASH },
    { "backslashkey", WM_BACKSLASH },
    { "backslash_key", WM_BACKSLASH },

     /* add more here */

    {"arrow_up", WM_UP },
    {"up_arrow", WM_UP },
    {"arrowup", WM_UP },
    {"uparrow", WM_UP },

    {"arrowdown", WM_DOWN },
    {"downarrow", WM_DOWN },
    {"down_arrow", WM_DOWN },
    {"arrow_down", WM_DOWN },

    {"arrowright", WM_RIGHT },
    {"rightarrow", WM_RIGHT },
    {"right_arrow", WM_RIGHT },
    {"arrow_right", WM_RIGHT },

    {"arrowleft", WM_LEFT },
    {"leftarrow", WM_LEFT },
    {"left_arrow", WM_LEFT },
    {"arrow_left", WM_LEFT },

     /* add more here */

    { "enter", WM_RETURN },
    { "return", WM_RETURN },

    { "f1", WM_F1 },
    { "f2", WM_F2 },
    { "f3", WM_F3 },
    { "f4", WM_F4 },
    { "f5", WM_F5 },
    { "f6", WM_F6 },
    { "f7", WM_F7 },
    { "f8", WM_F8 },
    { "f9", WM_F9 },
    { "f10", WM_F10 },
    { "f11", WM_F11 },
    { "f12", WM_F12 },

    { "space", WM_SPACE },
    { "spacebar", WM_SPACE },

    { "home", WM_HOME },
    { "end", WM_END },

    { "ins", WM_INSERT },
    { "insert", WM_INSERT },

    { "pause", WM_PAUSE },
    { "print", WM_PRINT },
    { "break", WM_BREAK },
    { "help", WM_HELP },
    { "menu", WM_MENU },

    { "pageup", WM_PAGE_UP },
    { "pgup", WM_PAGE_UP },
    { "page_up", WM_PAGE_UP },

    { "pagedown", WM_PAGE_DOWN },
    { "pgdn", WM_PAGE_DOWN },
    { "page_down", WM_PAGE_DOWN },

     /* add more here */
    { "kp0", WM_KP_0 },
    { "keypad0", WM_KP_0 },
    { "pad0", WM_KP_0 },

    { "kp1", WM_KP_1 },
    { "keypad1", WM_KP_1 },
    { "pad1", WM_KP_1 },

    { "kp2", WM_KP_2 },
    { "keypad2", WM_KP_2 },
    { "pad2", WM_KP_2 },

    { "kp3", WM_KP_3 },
    { "keypad3", WM_KP_3 },
    { "pad3", WM_KP_3 },

    { "kp4", WM_KP_4 },
    { "keypad4", WM_KP_4 },
    { "pad4", WM_KP_4 },

    { "kp5", WM_KP_5 },
    { "keypad5", WM_KP_5 },
    { "pad5", WM_KP_5 },

    { "kp6", WM_KP_6 },
    { "keypad6", WM_KP_6 },
    { "pad6", WM_KP_6 },

    { "kp7", WM_KP_7 },
    { "keypad7", WM_KP_7 },
    { "pad7", WM_KP_7 },

    { "kp8", WM_KP_8 },
    { "keypad8", WM_KP_8 },
    { "pad8", WM_KP_8 },

    { "kp9", WM_KP_9 },
    { "keypad9", WM_KP_9 },
    { "pad9", WM_KP_9 },

    { "kp_up", WM_KP_UP },
    { "keypad_up", WM_KP_UP },
    { "keypadup", WM_KP_UP },
    { "padup", WM_KP_UP },

    { "kp_down", WM_KP_DOWN },
    { "keypad_down", WM_KP_DOWN },
    { "keypaddown", WM_KP_DOWN },
    { "paddown", WM_KP_DOWN },

    { "kp_left", WM_KP_LEFT },
    { "keypad_left", WM_KP_LEFT },
    { "keypadleft", WM_KP_LEFT },
    { "padleft", WM_KP_LEFT },

    { "kp_right", WM_KP_RIGHT },
    { "keypad_right", WM_KP_RIGHT },
    { "keypadright", WM_KP_RIGHT },
    { "padright", WM_KP_RIGHT },
    
    { "kp_home", WM_KP_HOME },
    { "keypad_home", WM_KP_HOME },
    { "keypadhome", WM_KP_HOME },
    { "padhome", WM_KP_HOME },

    { "kp_end", WM_KP_END },
    { "keypad_end", WM_KP_END },
    { "keypadend", WM_KP_END },
    { "padend", WM_KP_END },

    { "kp_page_up", WM_KP_PAGE_UP },
    { "keypad_page_up", WM_KP_PAGE_UP },
    { "keypadpageup", WM_KP_PAGE_UP },
    { "padpageup", WM_KP_PAGE_UP },

    { "kp_page_down", WM_KP_PAGE_DOWN },
    { "keypad_page_down", WM_KP_PAGE_DOWN },
    { "keypadpagedown", WM_KP_PAGE_DOWN },
    { "padpagedown", WM_KP_PAGE_DOWN },

    { "kp_insert", WM_KP_INSERT },
    { "kp_ins", WM_KP_INSERT },
    { "keypad_insert", WM_KP_INSERT },
    { "keypad_ins", WM_KP_INSERT },
    { "keypadinsert", WM_KP_INSERT },
    { "keypadins", WM_KP_INSERT },
    { "padinsert", WM_KP_INSERT },
    { "padins", WM_KP_INSERT },

    { "kp_delete", WM_KP_DELETE },
    { "kp_del", WM_KP_DELETE },
    { "keypad_delete", WM_KP_DELETE },
    { "keypad_del", WM_KP_DELETE },
    { "keypaddelete", WM_KP_DELETE },
    { "keypaddel", WM_KP_DELETE },
    { "paddelete", WM_KP_DELETE },
    { "paddel", WM_KP_DELETE },

    { "kp_enter", WM_KP_ENTER },
    { "kp_return", WM_KP_ENTER },
    { "kpenter", WM_KP_ENTER },
    { "kpreturn", WM_KP_ENTER },
    { "keypadenter", WM_KP_ENTER },
    { "keypadreturn", WM_KP_ENTER },
    { "keypad_enter", WM_KP_ENTER },
    { "keypad_return", WM_KP_ENTER },
    { "padenter", WM_KP_ENTER },
    { "padreturn", WM_KP_ENTER },

    { "kp_divide", WM_KP_DIVIDE },
    { "kp_div", WM_KP_DIVIDE },
    { "keypad_divide", WM_KP_DIVIDE },
    { "keypad_div", WM_KP_DIVIDE },
    { "keypaddivide", WM_KP_DIVIDE },
    { "keypaddiv", WM_KP_DIVIDE },
    { "paddivide", WM_KP_DIVIDE },
    { "paddiv", WM_KP_DIVIDE },

    { "kp_multiply", WM_KP_MULTIPLY },
    { "kp_mul", WM_KP_MULTIPLY },
    { "keypad_multiply", WM_KP_MULTIPLY },
    { "keypad_mul", WM_KP_MULTIPLY },
    { "keypadmultiply", WM_KP_MULTIPLY },
    { "keypadmul", WM_KP_MULTIPLY },
    { "padmultiply", WM_KP_MULTIPLY },
    { "padmul", WM_KP_MULTIPLY },

    { "kp_subtract", WM_KP_SUBTRACT },
    { "kp_minus", WM_KP_SUBTRACT },
    { "keypad_subtract", WM_KP_SUBTRACT },
    { "keypad_minus", WM_KP_SUBTRACT },
    { "keypaddsubtract", WM_KP_SUBTRACT },
    { "keypaddminus", WM_KP_SUBTRACT },
    { "padsubtract", WM_KP_SUBTRACT },
    { "padminus", WM_KP_SUBTRACT },

    { "kp_add", WM_KP_ADD },
    { "kp_plus", WM_KP_ADD },
    { "keypad_add", WM_KP_ADD },
    { "keypad_plus", WM_KP_ADD },
    { "keypadadd", WM_KP_ADD },
    { "keypadplus", WM_KP_ADD },
    { "padadd", WM_KP_ADD },
    { "padplus", WM_KP_ADD },

    { "kp_decimal", WM_KP_DECIMAL },
    { "kp_dot", WM_KP_DECIMAL },
    { "keypad_decimal", WM_KP_DECIMAL },
    { "keypad_dot", WM_KP_DECIMAL },
    { "keypaddecimal", WM_KP_DECIMAL },
    { "keypaddot", WM_KP_DECIMAL },
    { "paddecimal", WM_KP_DECIMAL },
    { "paddot", WM_KP_DECIMAL },


};

static XCBKeyCode **WM_KEYBIND_GENERATE_KEYCODES_X11(XCBKeysym keysyms[], size_t num_keysyms)
{
    XCBKeyCode **codes = malloc(sizeof(XCBKeyCode *) * num_keysyms);

    if(!codes)
    {   return NULL;
    }

    size_t i;

    for(i = 0; i < num_keysyms; ++i)
    {
        XCBKeyCode *code = XCBKeySymbolsGetKeyCode(_wm.syms, keysyms[i]);

        if(!code)
        {   DebugWarn("Failed to get keycode for keysym %u\n", keysyms[i]);
        }

        codes[i] = code;
    }

    return codes;
}

static void WM_KEYBIND_GRAB_KEYCODE_X11(XCBKeyCode code, u16 mod)
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

int 
strcmp_lower(const char *a, const char *b)
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


static uint16_t WM_KEYBIND_FIND_MODIFIER_MASK_X11(const char *mod_str)
{
    int i;

    for(i = 0; i < LENGTH(mods_table); ++i)
    {
        if(strcmp_lower(mod_str, mods_table[i].name) == 0)
        {   return (uint16_t)mods_table[i].keycode;
        }
    }

    return 0;
}

static XCBKeysym WM_KEYBIND_FIND_KEYSYM_X11(const char *keysym_str)
{
    int i;

    for(i = 0; i < LENGTH(keycode_table); ++i)
    {
        if(strcmp_lower(keysym_str, keycode_table[i].name) == 0)
        {   return (XCBKeysym)keycode_table[i].keycode;
        }
    }

    return NoSymbol;
}

uint16_t 
WMKeybindModifierFromString(const char *mod_str)
{
    XCBKeysym sym = XStringToKeysym(mod_str);

    if(sym == NoSymbol)
    {   return WM_KEYBIND_FIND_MODIFIER_MASK_X11(mod_str);
    }
    else
    {
        /* if it's a valid keysym, check if it's a modifier */
        int i;

        for(i = 0; i < LENGTH(mods_table); ++i)
        {
            if(sym == mods_table[i].keycode)
            {   return (uint16_t)mods_table[i].keycode;
            }
        }
    }

    return 0;
}

XCBKeysym 
WMKeybindKeysymFromString(const char *keysym_str)
{
    XCBKeysym sym = XStringToKeysym(keysym_str);

    if(sym == NoSymbol)
    {   sym = WM_KEYBIND_FIND_KEYSYM_X11(keysym_str);
    }

    return sym;
}

int
WMKeybindCreate(Key *keybind_reference, u16 modifier_mask_x11, XCBKeysym keysyms[], size_t num_keysyms, void (*func)(const Key *, const Arg *), Arg arg, int lua_ref, bool on_press)
{
    if(!keybind_reference || !func)
    {   return EXIT_FAILURE;
    }

    if(!ASSERT(num_keysyms > 0 || modifier_mask_x11) || !ASSERT(keysyms || num_keysyms <= 0))
    {   return EXIT_FAILURE;
    }

    /* ALT_LEFT ALT_RIGHT */
    enum { WM_ALT_MULTIPLIER = 2 };
    /* CTRL_LEFT CTRL_RIGHT */
    enum { WM_CTRL_MULTIPLIER = 2 };
    /* SHIFT_LEFT SHIFT_RIGHT */
    enum { WM_SHIFT_MULTIPLIER = 2 };
    /* SUPER_LEFT SUPER_RIGHT */
    enum { WM_SUPER_MULTIPLIER = 2 };
    /* NUMLOCK */
    enum { WM_NUMLOCK_MULTIPLIER = 1 };
    /* CAPSLOCK */
    enum { WM_CAPSLOCK_MULTIPLIER = 1 };

    size_t alloced_keysyms = num_keysyms;

    /* substitude modifier mask */
    if(!num_keysyms)
    {
        keysyms = NULL;

        if(modifier_mask_x11 & WM_ALT)
        {   num_keysyms += WM_ALT_MULTIPLIER;
        }

        if(modifier_mask_x11 & WM_CTRL)
        {   num_keysyms += WM_CTRL_MULTIPLIER;
        }
        
        if(modifier_mask_x11 & WM_SHIFT)
        {   num_keysyms += WM_SHIFT_MULTIPLIER;
        }

        if(modifier_mask_x11 & WM_SUPER)
        {   num_keysyms += WM_SUPER_MULTIPLIER;
        }

        if(modifier_mask_x11 & WM_NUMLOCK)
        {   num_keysyms += WM_NUMLOCK_MULTIPLIER;
        }

        if(modifier_mask_x11 & WM_CAPSLOCK)
        {   num_keysyms += WM_CAPSLOCK_MULTIPLIER;
        }

        alloced_keysyms = num_keysyms;
    }

    keybind_reference->keysyms = malloc(sizeof(XCBKeysym) * alloced_keysyms);

    if(!keybind_reference->keysyms)
    {   
        DebugWarn("Failed to allocate memory for keybind keysyms");
        return EXIT_FAILURE;
    }

    /* keysyms exist default cpy it */
    if(keysyms)
    {
        memcpy(keybind_reference->keysyms, keysyms, sizeof(XCBKeysym) * alloced_keysyms);
        keybind_reference->num_keysyms = alloced_keysyms;
    }
    /* reconstruct keysyms */
    else
    {
        keysyms = keybind_reference->keysyms;

        num_keysyms = 0;

        if(modifier_mask_x11 & WM_ALT)
        {   
            keysyms[num_keysyms] = WM_ALT_LEFT;
            keysyms[num_keysyms + 1] = WM_ALT_RIGHT;

            num_keysyms += WM_ALT_MULTIPLIER;

            Debug("instantly cracked: WM_ALT");
        }

        if(modifier_mask_x11 & WM_CTRL)
        {   
            keysyms[num_keysyms] = WM_CTRL_LEFT;
            keysyms[num_keysyms + 1] = WM_CTRL_RIGHT;

            num_keysyms += WM_CTRL_MULTIPLIER;
            Debug("instantly cracked: WM_CTRL");
        }
        
        if(modifier_mask_x11 & WM_SHIFT)
        {   
            keysyms[num_keysyms] = WM_SHIFT_LEFT;
            keysyms[num_keysyms + 1] = WM_SHIFT_RIGHT;

            num_keysyms += WM_SHIFT_MULTIPLIER;
            Debug("instantly cracked: WM_SHIFT");
        }

        if(modifier_mask_x11 & WM_SUPER)
        {   
            keysyms[num_keysyms] = WM_SUPER_LEFT;
            keysyms[num_keysyms + 1] = WM_SUPER_RIGHT;

            num_keysyms += WM_SUPER_MULTIPLIER;
            Debug("instantly cracked: WM_SUPER");
        }

        if(modifier_mask_x11 & WM_NUMLOCK)
        {
            keysyms[num_keysyms] = WM_NUM_LOCK;

            num_keysyms += WM_NUMLOCK_MULTIPLIER;
            Debug("instantly cracked: WM_NUMLOCK");
        }

        if(modifier_mask_x11 & WM_CAPSLOCK)
        {   
            keysyms[num_keysyms] = WM_CAPS_LOCK;

            num_keysyms += WM_CAPSLOCK_MULTIPLIER;
            Debug("instantly cracked: WM_CAPSLOCK");
        }

        /* set to any modifier as we are declaring as key for single modifer key conversion */
        modifier_mask_x11 = XCBModMaskAny;
        keybind_reference->num_keysyms = alloced_keysyms;
    }

    keybind_reference->mod = modifier_mask_x11;
    keybind_reference->func = func;
    keybind_reference->arg = arg;
    keybind_reference->lua_ref = lua_ref;
    keybind_reference->on_press = on_press;

    return EXIT_SUCCESS;
}

int 
WMKeybindAdd(Key *keybind)
{
    if(!keybind || !keybind->func)
    {   return EXIT_FAILURE;
    }

    int status;

    status = GArrayPushBack(&keybinds, keybind);

    if(status != EXIT_SUCCESS)
    {   return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int 
WMKeybindRefresh(void)
{
    garray_i i, j, k;

    XCBUngrabKey(_wm.dpy, XCB_GRAB_ANY, XCB_MOD_MASK_ANY, _wm.root);

    for(i = GArrayStart(&keybinds); i < GArrayEnd(&keybinds); ++i)
    {
        Key *key = GArrayAt(&keybinds, i);
        XCBKeyCode **codes = WM_KEYBIND_GENERATE_KEYCODES_X11(key->keysyms, key->num_keysyms);

        if(!ASSERT(key))
        {   
            DebugError("key is NULL\n");
            continue;
        }

        if(!codes)
        {   
            DebugError("code is NULL\n");
            continue;
        }

        for(j = 0; j < key->num_keysyms; ++j)
        {
            if(!codes[j])
            {   
                DebugWarn("No keycode found for keysym %u\n", key->keysyms[j]);
                continue;
            }

            for(k = 0; codes[j][k] != XCB_NO_SYMBOL; ++k)
            {
                if(key->keysyms[j] == XCBKeySymbolsGetKeySym(_wm.syms, codes[j][k], 0))
                {   WM_KEYBIND_GRAB_KEYCODE_X11(codes[j][k], key->mod);
                }
            }

            free(codes[j]);
        }

        free(codes);
    }

    return EXIT_SUCCESS;
}

bool
WMKeybindHandler(u16 mod, XCBKeyCode code, bool pressed)
{
    /* ONLY use lowercase cause we dont know how to handle anything else */
    /* Only use upercase cause we dont know how to handle anything else
     * sym = XCBKeySymbolsGetKeySym(_wm.syms,  keydetail, 0);
     */
    /* This Could work MAYBE allowing for upercase and lowercase Keybinds However that would complicate things due to our ability to mask Shift
     * sym = XCBKeySymbolsGetKeySym(_wm.syms, keydetail, cleanstate); 
     */


    XCBKeysym sym = XCBKeySymbolsGetKeySym(_wm.syms, code, 0);

    /* Debug("%d", sym); */

    garray_i i;
    garray_i j;

    bool keybindRan = false;

    for(i = GArrayStart(&keybinds); i < GArrayEnd(&keybinds); ++i)
    {
        Key *key = GArrayAt(&keybinds, i);

        u16 mask = CLEANMASK(key->mod);

        for(j = 0; j < key->num_keysyms; ++j)
        {
            if(pressed != key->on_press)
            {   continue;
            }

            /* wrong mask */
            if(mask != mod)
            {   continue;
            }

            /* wrong keysym */
            if(sym != key->keysyms[j])
            {   continue;
            }

            /* TODO: This could be a issue with multi keybind hooking */
            key->func(key, &key->arg);
            keybindRan = true;
        }
    }

    return keybindRan;
}

void
WMKeybindRemoveAll(void)
{
    garray_i i;

    for(i = GArrayStart(&keybinds); i < GArrayEnd(&keybinds); ++i)
    {
        Key *key = GArrayAt(&keybinds, i);

        if(key->keysyms)
        {   free(key->keysyms);
        }
    }

    GArrayClear(&keybinds);

    XCBUngrabKey(_wm.dpy, XCB_GRAB_ANY, XCB_MOD_MASK_ANY, _wm.root);
}
