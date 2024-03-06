/**
 * @file xcb_trl.c
 * Code implementations.
 */


/* most of these implementations have been tanken from the tutorial, other websites, and or just straight reordering
 * some however have been taken from the source itself
 */


#include "xcb_trl.h"

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_event.h>
#include <xcb/xcbext.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xinerama.h>
#include <xcb/xcb_xrm.h>
#include <xcb/xcb_errors.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

static void (*_handler)(XCBDisplay *, XCBGenericError *) = NULL;

/* HELPER FUNCTION */
static XCBScreen *
screen_of_display(XCBDisplay *display, int screen)
{
    xcb_screen_iterator_t iter;

    iter = xcb_setup_roots_iterator(xcb_get_setup(display));
    for(; iter.rem; --screen, xcb_screen_next(&iter))
    {
        if(screen == 0)
        {   return iter.data;
        }
    }
    return NULL;
}

static void
_xcb_die(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (fmt[0] && fmt[strlen(fmt)-1] == ':')
    {   fputc(' ', stderr);
        perror(NULL);
    }
    else 
    {   fputc('\n', stderr);
    }
    exit(1);
}

static void
_xcb_err_handler(XCBDisplay *display, XCBGenericError *err)
{
    /* FIXME/TODO */
    if(!err || !display)
    {   return;
    }
    if(!_handler)
    {   
        if(err->error_code == XCB_NONE)
        {   return;
        }
        _xcb_die("error_code: [%d], major_code: [%d], minor_code: [%d]\n"
            "sequence: [%d], response_type: [%d], resource_id: [%d]\n"
            "full_sequence: [%d]\n"
            , 
            err->error_code, err->major_code, err->minor_code,
            err->sequence, err->response_type, err->resource_id,
            err->full_sequence
            );
    }
    /* this is unreachable if no handler is set */
    _handler(display, err);
}

XCBDisplay *
XCBOpenDisplay(const char *displayName, int *defaultScreenReturn)
{
    XCBDisplay *display = xcb_connect(displayName, defaultScreenReturn);
    /* We need to check return type cause xcb_connect never returns NULL (for some reason).
     * So we just check error, which requires display.
     */
    if(!display || xcb_connection_has_error(display))
    {   
        /* This frees the connection data that was allocated. */
        xcb_disconnect(display);
        return NULL;
    }
    return display;
}

void 
XCBCloseDisplay(XCBDisplay *display)
{
    /* Closes connection and frees resulting data. */
    xcb_disconnect(display);
}

int 
XCBDisplayNumber(XCBDisplay *display)
{
    /* gets the file descriptor AKA the connection number */
    return xcb_get_file_descriptor(display);
}

int
XCBConnectionNumber(XCBDisplay *display)
{
    /* gets the file descriptor AKA the connection number */
    return xcb_get_file_descriptor(display);
}

XCBScreen *
XCBScreenOfDisplay(XCBDisplay *display, int screen)
{
    return xcb_aux_get_screen(display, screen);
}

XCBScreen *
XCBDefaultScreenOfDisplay(XCBDisplay *display, int screen)
{
    return screen_of_display(display, screen);
}


int 
XCBScreenCount(XCBDisplay *display)
{
    return xcb_setup_roots_iterator (xcb_get_setup (display)).rem;
}

char *
XCBServerVendor(XCBDisplay *display)
{
    char *vendor = NULL;
    int length = 0;

    length = xcb_setup_vendor_length (xcb_get_setup (display));
    vendor = (char *)malloc (length + 1);
    if (vendor)
    {
        memcpy (vendor, xcb_setup_vendor (xcb_get_setup (display)), length);
        vendor[length] = '\0';
    }
    return vendor;
}


/* All of these use xcb_get_setup
 * The reason we can is because the access is essential O(1)
 * This is because xcb_get_setup() simply calls a error check and returns a adress
 * display->setup basically
 */

int 
XCBProtocolVersion(XCBDisplay *display)
{
    return xcb_get_setup(display)->protocol_major_version;
}

int 
XCBProtocolRevision(XCBDisplay *display)
{   return xcb_get_setup(display)->protocol_minor_version;
}

int
XCBVendorRelease(XCBDisplay *display)
{   return xcb_get_setup (display)->release_number;
}

int
XCBBitmapUnit(XCBDisplay *display)
{   return xcb_get_setup(display)->bitmap_format_scanline_unit;
}

int
XCBBitmapBitOrder(XCBDisplay *display)
{   return xcb_get_setup(display)->bitmap_format_bit_order;
}

int
XCBBitmapPad(XCBDisplay *display)
{   return xcb_get_setup(display)->bitmap_format_scanline_pad;
}

int
XCBImageByteOrder(XCBDisplay *display)
{   return xcb_get_setup(display)->image_byte_order;
}

const XCBSetup *
XCBGetSetup(XCBDisplay *display)
{
    return xcb_get_setup(display);
}

XCBScreen *
XCBGetScreen(XCBDisplay *display)
{
    return xcb_setup_roots_iterator(xcb_get_setup(display)).data;
}

XCBWindow
XCBRootWindow(XCBDisplay *display, int screen)
{
    /* Gets the screen structure */
    return xcb_aux_get_screen(display, screen)->root;
}

XCBWindow
XCBDefaultRootWindow(XCBDisplay *display, int screen)
{
    XCBScreen *scr = screen_of_display(display, screen);
    if(scr) 
    {   return scr->root;
    }
    return 0; /* AKA NULL; AKA we didnt find a screen */
}

u16 XCBDisplayWidth(XCBDisplay *display, int screen)
{
    /* Gets the screen structure */
    return xcb_aux_get_screen(display, screen)->width_in_pixels;
}
u16
XCBDisplayHeight(XCBDisplay *display, int screen)
{
    /* Gets the screen structure */
    return xcb_aux_get_screen(display, screen)->height_in_pixels;
}
u8
XCBDefaultDepth(XCBDisplay *display, int screen)
{
    /* Gets the screen structure */
    return xcb_aux_get_screen(display, screen)->root_depth;
}

XCBCookie
XCBSelectInput(XCBDisplay *display, XCBWindow window, u32 mask)
{
    return xcb_change_window_attributes(display, window, XCB_CW_EVENT_MASK, &mask);
}

XCBCookie
XCBSetInputFocus(XCBDisplay *display, XCBWindow window, u8 revert_to, XCBTimestamp time)
{
    return xcb_set_input_focus(display, revert_to, window, time);
}

XCBCookie
XCBChangeWindowAttributes(XCBDisplay *display, XCBWindow window, u32 mask, XCBWindowAttributes *window_attributes)
{
    return xcb_change_window_attributes_aux(display, window, mask, window_attributes);
}

u32
XCBBlackPixel(XCBDisplay *display, int screen)
{
    const XCBScreen *scr = screen_of_display(display, screen);
    if(scr)
    {   return scr->black_pixel;
    }
    return 0; /* AKA NULL; AKA we didnt find a screen */
}

u32
XCBWhitePixel(XCBDisplay *display, int screen)
{
    const XCBScreen *scr = screen_of_display(display, screen);
    if(scr)
    {   return scr->white_pixel;
    }
    return 0; /* AKA NULL; AKA we didnt find a screen */
}

void
XCBSync(XCBDisplay *display)
{ 
    /* "https://community.kde.org/Xcb"
     * The xcb equivalent of XSync() is xcb_aux_sync(), which is in xcb-utils.
     * The reason you won't find a sync function in libxcb is that there is no sync request in the X protocol. 
     * Calling XSync() or xcb_aux_sync() is equivalent to calling XGetInputFocus() and throwing away the reply.
     */
    xcb_aux_sync(display);
}




XCBCookie
XCBMoveWindow(XCBDisplay *display, XCBWindow window, i32 x, i32 y)
{
    const i32 values[] = { x, y };
    const u16 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    return xcb_configure_window(display, window, mask, values);
}

/* NOT TYPE SAFE
 * OVERFLOW CAN OCCUR ON u32 > i32
 */
XCBCookie
XCBMoveResizeWindow(XCBDisplay *display, XCBWindow window, i32 x, i32 y, u32 width, u32 height)
{
    const i32 values[] = { x, y, width, height };
    const u32 mask = XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT;
    return xcb_configure_window(display, window, mask, values);
}

XCBCookie
XCBRaiseWindow(XCBDisplay *display, XCBWindow window)
{
    const u32 values = { XCB_STACK_MODE_ABOVE };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    return xcb_configure_window(display, window, mask, &values);
}

XCBCookie
XCBMapRaised(XCBDisplay *display, XCBWindow window)
{
    xcb_map_window(display, window);
    return XCBRaiseWindow(display, window);
}

XCBCookie
XCBLowerWindow(XCBDisplay *display, XCBWindow window)
{
    const u32 values = { XCB_STACK_MODE_BELOW };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    return xcb_configure_window(display, window, mask, &values);
}

XCBCookie
XCBRaiseWindowIf(XCBDisplay *display, XCBWindow window)
{
    const u32 values = { XCB_STACK_MODE_TOP_IF };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    return xcb_configure_window(display, window, mask, &values);
}

XCBCookie
XCBLowerWindowIf(XCBDisplay *display, XCBWindow window)
{
    const u32 values = { XCB_STACK_MODE_BOTTOM_IF };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    return xcb_configure_window(display, window, mask, &values);
}

XCBCookie
XCBRaiseLowerWindow(XCBDisplay *display, XCBWindow window)
{
    const u32 values = { XCB_STACK_MODE_OPPOSITE };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    return xcb_configure_window(display, window, mask, &values);
}

XCBCookie
XCBSetWindowBorderWidth(XCBDisplay *display, XCBWindow window, u32 border_width)
{
    const u32 values = { border_width };
    const u32 mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
    return xcb_configure_window(display, window, mask, &values);
}

XCBCookie
XCBSetSibling(XCBDisplay *display, XCBWindow window, XCBWindow sibling)
{
    const u32 values = { sibling };
    const u32 mask = XCB_CONFIG_WINDOW_SIBLING;
    return xcb_configure_window(display, window, mask, &values);
}

XCBWindowAttributesCookie
XCBGetWindowAttributesCookie(XCBDisplay *display, XCBWindow window)
{
    return xcb_get_window_attributes(display, window);
}

XCBWindowAttributesReply *
XCBGetWindowAttributesReply(XCBDisplay *display, XCBWindowAttributesCookie cookie)
{
    XCBGenericError *err = NULL;
    XCBWindowAttributesReply *reply = NULL;
    reply = xcb_get_window_attributes_reply(display, cookie, &err);
    _xcb_err_handler(display, err);
    return reply;
}

XCBGeometryCookie
XCBGetWindowGeometryCookie(XCBDisplay *display, XCBWindow window)
{
    return xcb_get_geometry(display, window);
}

XCBGeometry *
XCBGetWindowGeometryReply(XCBDisplay *display, XCBGeometryCookie cookie)
{
    XCBGenericError *err = NULL;
    XCBGeometry *reply = NULL;
    reply = xcb_get_geometry_reply(display, cookie, &err);
    _xcb_err_handler(display, err);
    return reply;
}

XCBAtomCookie
XCBInternAtomCookie(XCBDisplay *display, const char *name, int only_if_exists)
{
    return xcb_intern_atom(display, only_if_exists, strlen(name), name);
}

XCBAtom
XCBInternAtomReply(XCBDisplay *display, XCBAtomCookie cookie)
{
    XCBGenericError *err = NULL;
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(display, cookie, &err);
    _xcb_err_handler(display, err);
    xcb_atom_t atom = 0;
    if(reply)
    {
        atom = reply->atom;
        free(reply);
    }
    return atom;
}



XCBPixmap
XCBCreatePixmap(XCBDisplay *display, XCBWindow root, u16 width, u16 height, u8 depth)
{
    const XCBPixmap id = xcb_generate_id(display);
    xcb_create_pixmap(display, depth, id, root, width, height);
    return id;
}

/* Cursors */

XCBCursor
XCBCreateFontCursor(XCBDisplay *display, int shape)
{
    /* FORMAT IN RGB: (AKA RED GREEN BLUE) */
    /* fg = foreground; bg = background */
    const u16 fgred = 0;
    const u16 fggreen = 0;
    const u16 fgblue = 0;

    const u16 bgred = 0;
    const u16 bggreen = 0;
    const u16 bgblue = 0;

    const xcb_font_t font = xcb_generate_id(display);

    (void)xcb_open_font(display, font, strlen("cursor"), "cursor");

    const xcb_cursor_t id = xcb_generate_id(display);
    (void)xcb_create_glyph_cursor(display, id, font, font, shape, shape + 1,
                            fgred, fggreen, fgblue,
                            bgred, bggreen, bgblue);
    return id;
}

XCBCookie
XCBFreeCursor(XCBDisplay *display, XCBCursor cursor)
{
    return xcb_free_cursor(display, cursor);
}

XCBCookie
XCBDefineCursor(XCBDisplay *display, XCBWindow window, XCBCursor id)
{   
    const u32 mask = XCB_CW_CURSOR;
    return xcb_change_window_attributes(display, window, mask, &id);
}

XCBCookie
XCBOpenFont(XCBDisplay *display, XCBFont id, const char *name)
{
    const u16 len = strlen(name);
    return xcb_open_font(display, id, len, name);
}

XCBCookie
XCBCloseFont(XCBDisplay *display, XCBFont id)
{
    return xcb_close_font(display, id);
}


/* text property textproperty */

XCBTextPropertyCookie
XCBGetTextPropertyCookie(XCBDisplay *display, XCBWindow window, XCBAtom property)
{
    return xcb_icccm_get_text_property(display, window, property);
}

int 
XCBGetTextPropertyReply(XCBDisplay *display, XCBTextPropertyCookie cookie, XCBTextProperty *reply_return)
{
    XCBGenericError *err = NULL;
    int status = 0;
    status = xcb_icccm_get_text_property_reply(display, cookie, reply_return, &err);
    _xcb_err_handler(display, err);
    return status;
}
int
XCBFreeTextProperty(XCBTextProperty *prop)
{   
    xcb_icccm_get_text_property_reply_wipe(prop);
    return 1;
}

int 
XCBFlush(XCBDisplay *display)
{
    /* This locks the XServer thread (pthread mutix lock).
     * and writes the buffer to the server.
     * RETURN 0 on Success.
     * RETURN 1 on Failure.
     */
    return xcb_flush(display);
}

u32
XCBGetMaximumRequestLength(XCBDisplay *display)
{
    /* <Straight from the documentation.>
     *
     * In the absence of the BIG-REQUESTS extension, returns the
     * maximum request length field from the connection setup data, which
     * may be as much as 65535. If the server supports BIG-REQUESTS, then
     * the maximum request length field from the reply to the
     * BigRequestsEnable request will be returned instead.
     *
     * Note that this length is measured in four-byte units, making the
     * theoretical maximum lengths roughly 256kB without BIG-REQUESTS and
     * 16GB with.
     */
    return xcb_get_maximum_request_length(display);
}

int
XCBCheckDisplayError(XCBDisplay *display)
{   return xcb_connection_has_error(display);
}
int 
XCBHasConnectionError(XCBDisplay *display)
{   return xcb_connection_has_error(display);
}
int 
XCBCheckConnectionError(XCBDisplay *display)
{
    return xcb_connection_has_error(display);
}
int 
XCBHasDisplayError(XCBDisplay *display)
{   return xcb_connection_has_error(display);
}

int
XCBSetErrorHandler(void (*error_handler)(XCBDisplay *, XCBGenericError *))
{   return !!(_handler = error_handler);
}

void
XCBSetIOErrorHandler(XCBDisplay *display, void *IOHandler)
{   /* not implemented */
    (void)display;
    (void)IOHandler;
    return;
}

char *
XCBGetErrorText(XCBDisplay *display)
{
    /* TODO */
    return "";
}






/* events */





int 
XCBNextEvent(XCBDisplay *display, XCBGenericEvent **event_return) 
{
    /* waits till next event happens before returning */
    return !!(*event_return = xcb_wait_for_event(display));
}

XCBGenericEvent *
XCBWaitForEvent(XCBDisplay *display)
{
    /* waits till next event happens before returning */
    return xcb_wait_for_event(display);
}

XCBGenericEvent *
XCBPollForEvent(XCBDisplay *display)
{
    /* TODO */
    /* If I/O error do something */
   return  xcb_poll_for_event(display);
}

XCBGenericEvent *
XCBPollForQueuedEvent(XCBDisplay *display)
{
    return xcb_poll_for_queued_event(display);
}

XCBCookie
XCBGrabKey(XCBDisplay *display, XCBKeyCode keycode, u16 modifiers, XCBWindow grab_window, u8 owner_events, u8 pointer_mode, u8 keyboard_mode)
{
    return xcb_grab_key(display, owner_events, grab_window, keycode, modifiers, pointer_mode, keyboard_mode);
}

XCBCookie 
XCBUngrabKey(XCBDisplay *display, XCBKeyCode key, u16 modifiers, XCBWindow grab_window)
{
    return xcb_ungrab_key(display, key, grab_window, modifiers);
}

XCBCookie
XCBUngrabButton(XCBDisplay *display, uint8_t button, uint16_t modifier, XCBWindow window)
{
    return xcb_ungrab_button(display, button, window, modifier);
}

XCBCookie
XCBGrabButton(
        XCBDisplay *display, 
        u8 button, 
        u16 modifiers, 
        XCBWindow grab_window, 
        u8 owner_events, 
        u16 event_mask, 
        u8 pointer_mode,
        u8 keyboard_mode,
        XCBWindow confine_to,
        XCBCursor cursor)
{
    return xcb_grab_button(display, owner_events, grab_window, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, button, modifiers);
}

int
XCBDisplayKeyCodes(XCBDisplay *display, int *min_keycode_return, int *max_keycode_return)
{
    const XCBSetup *setup = xcb_get_setup(display);
    *min_keycode_return = setup->min_keycode;
    *max_keycode_return = setup->max_keycode;
    return 1;
}

int 
XCBDisplayKeycodes(XCBDisplay *display, int *min_keycode_return, int *max_keycode_return)
{
    const XCBSetup *setup = xcb_get_setup(display);
    *min_keycode_return = setup->min_keycode;
    *max_keycode_return = setup->max_keycode;
    return 1;
}

XCBKeyCode *
XCBGetKeyCodes(XCBDisplay *display, XCBKeysym keysym)
{
    xcb_key_symbols_t *keysyms;
	xcb_keycode_t *keycode;

	if (!(keysyms = xcb_key_symbols_alloc(display)))
		return NULL;

	keycode = xcb_key_symbols_get_keycode(keysyms, keysym);
	xcb_key_symbols_free(keysyms);

	return keycode;
}

XCBKeycode *
XCBGetKeycodes(XCBDisplay *display, XCBKeysym keysym)
{
    return XCBGetKeyCodes(display, keysym);
}

XCBKeyCode *
XCBKeySymbolsGetKeyCode(
        XCBKeySymbols *symbols, 
        XCBKeysym keysym)
{
    return xcb_key_symbols_get_keycode(symbols, keysym);
}

XCBKeycode *
XCBKeySymbolsGetKeycode(
        XCBKeySymbols *symbols,
        XCBKeysym keysym
        )
{
    return xcb_key_symbols_get_keycode(symbols, keysym);
}

XCBKeysym
XCBKeySymbolsGetKeySym(
        XCBKeySymbols *symbols,
        XCBKeyCode keycode,
        uint8_t column
        )
{   
    return xcb_key_symbols_get_keysym(symbols, keycode, column);
}

XCBKeySymbols *
XCBKeySymbolsAlloc(XCBDisplay *display)
{
    return xcb_key_symbols_alloc(display);
}

void
XCBKeySymbolsFree(XCBKeySymbols *keysyms)
{
    xcb_key_symbols_free(keysyms);
}

XCBKeyboardMappingCookie 
XCBGetKeyboardMappingCookie(XCBDisplay *display, XCBKeyCode first_keycode, u8 count)
{
    return xcb_get_keyboard_mapping(display, first_keycode, count);
}

XCBKeyboardMapping *
XCBGetKeyboardMappingReply(XCBDisplay *display, XCBKeyboardMappingCookie cookie)
{
    XCBGenericError *err = NULL;
    XCBKeyboardMapping *reply = xcb_get_keyboard_mapping_reply(display, cookie, &err);
    _xcb_err_handler(display, err);
    return reply;
}

XCBPointerCookie
XCBQueryPointerCookie(XCBDisplay *display, XCBWindow window)
{
    return xcb_query_pointer(display, window);
}

XCBPointerReply *
XCBQueryPointerReply(XCBDisplay *display, XCBPointerCookie cookie)
{
    XCBGenericError *err = NULL;
    XCBPointerReply *reply = xcb_query_pointer_reply(display, cookie, &err);
    _xcb_err_handler(display, err);
    return reply;
}


XCBCookie
XCBMapWindow(XCBDisplay *display, XCBWindow window)
{
    return xcb_map_window(display, window);
}

XCBWindow 
XCBCreateWindow(
        XCBDisplay *display, 
        XCBWindow parent, 
        i16 x, 
        i16 y, 
        u16 width, 
        u16 height, 
        u16 border_width, 
        u8 depth, 
        unsigned int class, 
        XCBVisual visual, 
        u32 valuemask, 
        const u32 *value_list)
{
    const XCBWindow id = xcb_generate_id(display);
    const void *used = NULL;
    xcb_create_window(display, depth, id, parent, x, y, width, height, border_width, 
    class, visual, valuemask, used);
    return id;
}

XCBWindow
XCBCreateSimpleWindow(
        XCBDisplay *display,
        XCBWindow parent,
        i16 x,
        i16 y,
        u16 width,
        u16 height,
        u16 border_width,
        uint32_t border_color,
        uint32_t background_color
        )
{
    const XCBWindow id = xcb_generate_id(display);
    const XCBVisual visual = XCBGetScreen(display)->root_visual;
    const u8  class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    const u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL;
    const u32 color[2] = { background_color, border_color };
    xcb_create_window(display, XCB_COPY_FROM_PARENT, id, parent, x, y, width, height, border_width, class, visual, mask, &color);
    return id;
}


XCBGC
XCBCreateGC(XCBDisplay *display, XCBDrawable drawable, 
u32 valuemask, const void *valuelist)
{
    const XCBGC id = xcb_generate_id(display);
    xcb_create_gc(display, id, drawable, valuemask, valuelist);
    return id;
}

int
XCBSetLineAttributes(XCBDisplay *display, XCBGC gc, u32 linewidth, u32 linestyle, u32 capstyle, u32 joinstyle)
{
    u32 gcvalist[4];

    u8 i = 0; /* This is the index; u8 just to save some bytes */
    u32 mask = 0;
    mask |= XCB_GC_LINE_WIDTH;
    gcvalist[i++] = linewidth;

    mask |= XCB_GC_LINE_STYLE;
    gcvalist[i++] = linestyle;

    mask |= XCB_GC_CAP_STYLE;
    gcvalist[i++] = capstyle;

    mask |= XCB_GC_JOIN_STYLE; 
    gcvalist[i++] = joinstyle;


    /* This returns a cookie but changing the gc isnt doesnt really require a reply as you are directly manupulating your own gc
     */
    xcb_change_gc(display, gc, mask, gcvalist);
    /* X11 src/SetLStyle.c always returns 1 (for some reason) */
    return 1;
}

XCBCookie
XCBChangeProperty(XCBDisplay *display, XCBWindow window, XCBAtom property, XCBAtom type, u8 format, u8 mode, const void *data, u32 nelements)
{
    return xcb_change_property(display, mode, window, property, type, format, nelements, data);
}

XCBCookie
XCBDeleteProperty(XCBDisplay *display, XCBWindow window, XCBAtom property)
{
    return xcb_delete_property(display, window, property);
}

XCBCookie
XCBConfigureWindow(
        XCBDisplay *display, 
        XCBWindow window,
        u16 value_mask,
        XCBWindowChanges *changes)
{
    return xcb_configure_window_aux(display, window, value_mask, changes);
}

XCBCookie
XCBSetClassHint(XCBDisplay *display, XCBWindow window, const char *class_name)
{
    const int len = strlen(class_name);
    return xcb_icccm_set_wm_class(display, window, len, class_name);
}


XCBCookie
XCBChangeGC(XCBDisplay *display, XCBGC gc, u32 valuemask, const void *valuelist)
{
    return xcb_change_gc(display, gc, valuemask, valuelist);
}

XCBCookie
XCBDrawPoint(XCBDisplay *display, u8 coordinatemode, XCBDrawable drawable, XCBGC gc, uint32_t points_len, XCBPoint *points)
{
    return xcb_poly_point(display, coordinatemode, drawable, gc, points_len, points);
}

int
XCBDiscardReply(XCBDisplay *display, XCBCookie cookie)
{
    xcb_discard_reply(display, cookie.sequence);
    return cookie.sequence;
}

int
XCBThrowAwayReply(XCBDisplay *display, XCBCookie cookie)
{
    xcb_discard_reply(display, cookie.sequence);
    return cookie.sequence;
}

/* dumb stuff */

void 
XCBPrefetchMaximumRequestLength(XCBDisplay *display)
{
    /**
     * @brief Prefetch the maximum request length without blocking.
     * @param c The connection to the X server.
     *
     * Without blocking, does as much work as possible toward computing
     * the maximum request length accepted by the X server.
     *
     * Invoking this function may cause a call to xcb_big_requests_enable,
     * but will not block waiting for the reply.
     * xcb_get_maximum_request_length will return the prefetched data
     * after possibly blocking while the reply is retrieved.
     *
     * Note that in order for this function to be fully non-blocking, the
     * application must previously have called
     * xcb_prefetch_extension_data(c, &xcb_big_requests_id) and the reply
     * must have already arrived.
     */
    xcb_prefetch_maximum_request_length(display);
}


/* ICCCM */

XCBGetPropertyCookie
XCBGetWMProtocolsCookie(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBAtom protocol)
{
    return xcb_icccm_get_wm_protocols(display, window, protocol);
}

int
XCBGetWMProtocolsReply(
        XCBDisplay *display, 
        XCBGetPropertyCookie cookie,
        XCBGetWMProtocol *protocol_return
        )
{
    XCBGenericError *err = NULL;
    int ret;
    ret = xcb_icccm_get_wm_protocols_reply(display, cookie, protocol_return, &err);
    _xcb_err_handler(display, err);
    return ret;

}

void
XCBWipeGetWMProtocolsReply(
        XCBGetWMProtocol *protocols)
{
    xcb_icccm_get_wm_protocols_reply_wipe(protocols);
}



//xcb_generic_event_t *xcb_poll_for_queued_event(xcb_connection_t *c);
