#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#define XK_LATIN1
#define XK_LATIN2
#define XK_LATIN3
#define XK_LATIN4
#define XK_CYRILLIC
#define XK_GREEK
#define XK_ARMENIAN
#include <X11/keysymdef.h>

#include "xcb_keysym.h"
#include "xcb_trl.h"
#include "khash.h"
#include "__private__xcb__err__.h"
#include "__private__xcb__utils__.h"


/* Private declaration */
enum tag_t 
{
    TAG_COOKIE,
    TAG_VALUE
};

typedef struct _XCBKeySymbols _XCBKeySymbols;

struct _XCBKeySymbols
{
    xcb_connection_t *c;
    enum tag_t     tag;
    union 
    {
        xcb_get_keyboard_mapping_cookie_t cookie;
        xcb_get_keyboard_mapping_reply_t *reply;
    } u;
};


XCBKeyCode *
XCBGetKeyCodes(
        XCBDisplay *display, 
        XCBKeysym keysym
        )
{
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    xcb_key_symbols_t *keysyms;
	xcb_keycode_t *keycode;

	if (!(keysyms = xcb_key_symbols_alloc(display)))
    {   return NULL;
    }

	keycode = xcb_key_symbols_get_keycode(keysyms, keysym);
	xcb_key_symbols_free(keysyms);

	return keycode;
}


XCBKeycode *
XCBGetKeycodes(
        XCBDisplay *display, 
        XCBKeysym keysym
        )
{
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    return XCBGetKeyCodes(display, keysym);
}


XCBKeyCode *
XCBKeySymbolsGetKeyCode(
        XCBKeySymbols *symbols, 
        XCBKeysym keysym
        )
{
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    return xcb_key_symbols_get_keycode(symbols, keysym);
}


XCBKeycode *
XCBKeySymbolsGetKeycode(
        XCBKeySymbols *symbols,
        XCBKeysym keysym
        )
{
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    return xcb_key_symbols_get_keycode(symbols, keysym);
}

XCBKeysym
XCBKeySymbolsGetKeySym(
        XCBKeySymbols *symbols,
        XCBKeyCode keycode,
        enum XCBKeysymColumn column
        )
{   
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    return xcb_key_symbols_get_keysym(symbols, keycode, column);
}

XCBKeySymbols *
XCBKeySymbolsAlloc(
        XCBDisplay *display
        )
{
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    return xcb_key_symbols_alloc(display);
}

void
XCBKeySymbolsFree(
        XCBKeySymbols *keysyms
        )
{
    XCBCookie ret = { .sequence = 0 };
    _xcb_push_func(ret);

    xcb_key_symbols_free(keysyms);
}
