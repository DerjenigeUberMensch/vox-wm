/**
 * @file xcb_trl.c
 * Code implementations.
 */


/* most of these implementations have been tanken from the tutorial, other websites, and or just straight reordering
 * some however have been taken from the source itself
 */



/*
 * NOTE: while xcb claims to be big speed it doesnt hesitate to call malloc.
 */



/*
 * TODO: xcb for some reason doesnt have a 32bit version of alot of the xlib to xcb ports for some reason... so we have to do it ourselves
 */

#include "xcb_trl.h"

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xcbext.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xinerama.h>
#include <xcb/xcb_xrm.h>
#include <xcb/xcb_errors.h>


/* error codes */
#include <X11/X.h>
#include <X11/Xproto.h>

#include <stdio.h>
#include <string.h>
#include <limits.h> /* UINT_MAX */
#include <stdarg.h>

/** Number of elements in this structure */
#define XCB_ICCCM_NUM_WM_HINTS_ELEMENTS 9




typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

static void (*_handler)(XCBDisplay *, XCBGenericError *) = NULL;


#ifdef XCB_TRL_ENABLE_DEBUG
#define DBG             1
#define _fn             __func__
#endif
#define _XCB_MANUAL_DEBUG(fmt, ...)             (fprintf(stderr, "_XCB_DEBUG_ [%s:%d] by %s(): " fmt "\n", __FILE__,__LINE__,__func__,__VA_ARGS__))
#define _XCB_MANUAL_DEBUG0(X)                   (fprintf(stderr, "_XCB_DEBUG_ [%s:%d] by %s(): " X "\n", __FILE__, __LINE__, __func__))

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

#ifdef DBG


typedef struct _xcb_caller _xcb_caller;

struct _xcb_caller 
{
    XCBCookie id;
    char *name;
};

#define MAX_DEBUG_LIMIT     32768   /* the number means nothing its just some random big enough number for most use cases */
                
_xcb_caller _xcb_funcs[MAX_DEBUG_LIMIT];
uint32_t rear = -1;
uint32_t front = -1;


static void
_xcb_jmpck(XCBDisplay *d, XCBGenericError *err)
{
    fprintf(stderr, "_XCB_DEBUG_\n");
    fprintf(stderr, "%s %s\n", XCBErrorCodeText(err->error_code), XCBErrorMajorCodeText(err->major_code));
    fprintf(stderr, 
            "error_code:    [%d]\n"
            "major_code:    [%d]\n"
            "minor_code:    [%d]\n"
            "sequence:      [%d]\n"
            "response_type: [%d]\n"
            "resource_id:   [%d]\n"
            "full_sequence: [%d]\n"
              ,
           err->error_code, err->major_code, err->minor_code, 
           err->sequence, err->response_type, err->resource_id, 
           err->full_sequence);

}

static void _xcb_push_func(XCBCookie cookie, char *func);
static void _xcb_pop_func(XCBCookie cookie);

static void
_xcb_push_func(XCBCookie cookie, char *func)
{
    _xcb_caller item;
    item.id = cookie;
    item.name = func;

    if(rear == MAX_DEBUG_LIMIT - 1)
    {
        _XCB_MANUAL_DEBUG0("overflow?");
    }
    else
    {
        if(front == -1)
        {
            front = 0;
            rear += 1;
            _xcb_funcs[rear] = item;
        }
    }
}
static void
_xcb_pop_func(XCBCookie cookie)
{
    if(front == -1 || front > rear)
    {
        _XCB_MANUAL_DEBUG0("underflow?");
    }
    else
    {
        front += 1;
    }
}

static void
_xcb_show_call_stack(void)
{
    if(front == -1)
    {   _XCB_MANUAL_DEBUG0("No stack to show");
    }
    else
    {
        for(int i = front; i <= rear; ++i)
        {   _XCB_MANUAL_DEBUG("%s", _xcb_funcs[i].name);
        }
    }
}

void  
XCBBreakPoint(void) 
{
}

void
XCBDebugShowCallStack(void)
{
    _xcb_show_call_stack();
}

#else

void
XCBDebugShowCallStack(void)
{
    _XCB_MANUAL_DEBUG0("Debugging is not enabled in this session.");
}

#endif




static void
_xcb_err_handler(XCBDisplay *display, XCBGenericError *err)
{
    /* FIXME/TODO */
    if(!err || !display)
    {   return;
    }
    if(_handler)
    {   _handler(display, err);
    }
    else
    {
#ifdef DBG
    _xcb_jmpck(display, err);
#endif
    }
    free(err);
    err = NULL;
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
XCBDisplay *
XCBOpenConnection(const char *displayName, int *defaultScreenReturn)
{
    return xcb_connect(displayName, defaultScreenReturn);
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
    XCBCookie ret = xcb_change_window_attributes(display, window, XCB_CW_EVENT_MASK, &mask);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBSetInputFocus(XCBDisplay *display, XCBWindow window, u8 revert_to, XCBTimestamp tim)
{
    XCBCookie ret = xcb_set_input_focus(display, revert_to, window, tim);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBChangeWindowAttributes(XCBDisplay *display, XCBWindow window, u32 mask, XCBWindowAttributes *window_attributes)
{
    XCBCookie ret = xcb_change_window_attributes_aux(display, window, mask, window_attributes);
#ifdef DBG
#endif
    return ret;
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

void
XCBSyncf(XCBDisplay *display)
{
    /* analagous to XSync(display, True) */
    XCBSync(display);
    XCBGenericEvent *ev = NULL;
    while(((ev = XCBPollForQueuedEvent(display))))
    {   
        if(!ev->response_type)
        {   _xcb_err_handler(display, (xcb_generic_error_t *)ev);
        }
        else
        {   free(ev);
        }
    }
}




XCBCookie
XCBMoveWindow(XCBDisplay *display, XCBWindow window, i32 x, i32 y)
{
    const i32 values[4] = { x, y };
    const u16 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    XCBCookie ret = xcb_configure_window(display, window, mask, values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBMoveResizeWindow(XCBDisplay *display, XCBWindow window, i32 x, i32 y, u32 width, u32 height)
{
    /* fucks up sometimes so this will do */
    XCBMoveWindow(display, window, x, y);
    XCBCookie ret = XCBResizeWindow(display, window, width, height);
    return ret;
}

XCBCookie
XCBResizeWindow(XCBDisplay *display, XCBWindow window, u32 width, u32 height)
{
    const u32 values[4] = { width, height };
    const u32 mask = XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT;
    XCBCookie ret = xcb_configure_window(display, window, mask, values);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBRaiseWindow(XCBDisplay *display, XCBWindow window)
{
    const u32 values[1] = { XCB_STACK_MODE_ABOVE };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBMapRaised(XCBDisplay *display, XCBWindow window)
{
    xcb_map_window(display, window);
    XCBCookie ret = XCBRaiseWindow(display, window);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBLowerWindow(XCBDisplay *display, XCBWindow window)
{
    const u32 values[1] = { XCB_STACK_MODE_BELOW };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBRaiseWindowIf(XCBDisplay *display, XCBWindow window)
{
    const u32 values[1] = { XCB_STACK_MODE_TOP_IF };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBLowerWindowIf(XCBDisplay *display, XCBWindow window)
{
    const u32 values[1] = { XCB_STACK_MODE_BOTTOM_IF };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBRaiseLowerWindow(XCBDisplay *display, XCBWindow window)
{
    const u32 values[1] = { XCB_STACK_MODE_OPPOSITE };
    const u32 mask = XCB_CONFIG_WINDOW_STACK_MODE;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBSetWindowBorderWidth(XCBDisplay *display, XCBWindow window, u32 border_width)
{
    const u32 values[1] = { border_width };
    const u32 mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBSetSibling(XCBDisplay *display, XCBWindow window, XCBWindow sibling)
{
    const u32 values[1] = { sibling };
    const u32 mask = XCB_CONFIG_WINDOW_SIBLING;
    XCBCookie ret = xcb_configure_window(display, window, mask, &values);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBGetWindowAttributesCookie(XCBDisplay *display, XCBWindow window)
{
    const xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(display, window);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBGetWindowAttributes *
XCBGetWindowAttributesReply(XCBDisplay *display, XCBCookie cookie)
{
    XCBGenericError *err = NULL;
    XCBGetWindowAttributes *reply = NULL;
    const xcb_get_window_attributes_cookie_t cookie1 = { .sequence = cookie.sequence };
    reply = xcb_get_window_attributes_reply(display, cookie1, &err);
    if(err)
    {
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

XCBCookie
XCBGetWindowGeometryCookie(XCBDisplay *display, XCBWindow window)
{
    const xcb_get_geometry_cookie_t cookie = xcb_get_geometry(display, window);
    XCBCookie ret = { .sequence = cookie.sequence };
    return ret;
}

XCBGeometry *
XCBGetWindowGeometryReply(XCBDisplay *display, XCBCookie cookie)
{
    XCBGenericError *err = NULL;
    XCBGeometry *reply = NULL;
    const xcb_get_geometry_cookie_t cookie1 = { .sequence = cookie.sequence };
    reply = xcb_get_geometry_reply(display, cookie1, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}


XCBCookie
XCBGetGeometryCookie(
        XCBDisplay *display,
        XCBWindow window)
{
    XCBCookie ret = XCBGetWindowGeometryCookie(display, window);
#ifdef DBG
#endif
    return ret;
}

XCBGeometry *
XCBGetGeometryReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGeometry *ret = XCBGetWindowGeometryReply(display, cookie);
    return ret;
}


XCBCookie
XCBInternAtomCookie(XCBDisplay *display, const char *name, int only_if_exists)
{
    const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(display, only_if_exists, strlen(name), name);
    XCBCookie ret = { .sequence = cookie.sequence };

#ifdef DBG
#endif

    return ret;
}

XCBAtom
XCBInternAtomReply(XCBDisplay *display, XCBCookie cookie)
{
    XCBGenericError *err = NULL;
    const xcb_intern_atom_cookie_t cookie1 = { .sequence = cookie.sequence };
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(display, cookie1, &err);
    if(err)
    {
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return 0;
    }
    const xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

XCBCookie
XCBGetTransientForHintCookie(
        XCBDisplay *display, 
        XCBWindow win)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(display, 0, win, XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
    XCBCookie ret = { .sequence = cookie.sequence };
    return ret;
}

u8
XCBGetTransientForHintReply(
        XCBDisplay *display,
        XCBCookie cookie,
        XCBWindow *win
        )
{
    XCBGenericError *err = NULL;
    xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };
    xcb_get_property_reply_t *reply = xcb_get_property_reply(display, cookie1, &err);
    u8 status = 1;

    if(!reply || reply->type != XCB_ATOM_WINDOW || reply->format != 32 || !reply->length)
    {   status = 0;
    }
    else
    {
        if(win)
        {   *win = *((xcb_window_t *) xcb_get_property_value(reply));
        }
        else
        {   
            status = 0;
            #ifdef DBG
                _XCB_MANUAL_DEBUG0("No adress specified in the 'win' field.");
                XCBBreakPoint();
            #endif
        }
    }

    if(err)
    {   _xcb_err_handler(display, err);
        return 0;
    }
    free(reply);
    return status;
}

XCBCookie
XCBGetPropertyCookie(
        XCBDisplay *display,
        XCBWindow window,
        XCBAtom property,
        uint32_t long_offset,
        uint32_t long_length,
        uint8_t _delete,
        XCBAtom req_type
        )
{
    const xcb_get_property_cookie_t cookie = xcb_get_property(display, _delete, window, property, req_type, long_offset, long_length);
    XCBCookie ret = {.sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBCookie 
XCBGetWindowPropertyCookie(
        XCBDisplay *display,
        XCBWindow window,
        XCBAtom property,
        uint32_t long_offset,
        uint32_t long_length,
        uint8_t _delete,
        XCBAtom req_type
        )
{
    const xcb_get_property_cookie_t cookie = xcb_get_property(display, _delete, window, property, req_type, long_offset, long_length);
    XCBCookie ret = {.sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBWindowProperty *
XCBGetWindowPropertyReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    const xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };
    xcb_get_property_reply_t *reply = xcb_get_property_reply(display, cookie1, &err);
    if(err)
    {
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

void *
XCBGetWindowPropertyValue(XCBWindowProperty *reply)
{
    return xcb_get_property_value(reply);
}

u32
XCBGetWindowPropertyValueSize(XCBWindowProperty *reply)
{
    return xcb_get_property_value_length(reply);
}

u32
XCBGetWindowPropertyValueLength(XCBWindowProperty *reply, size_t size)
{
    return xcb_get_property_value_length(reply) / (size);
}

XCBWindowProperty *
XCBGetPropertyReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    const xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };
    xcb_get_property_reply_t *reply = xcb_get_property_reply(display, cookie1, &err);
    if(err)
    {
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

void *
XCBGetPropertyValue(XCBWindowProperty *reply)
{
    return xcb_get_property_value(reply);
}

u32
XCBGetPropertyValueSize(
        XCBWindowProperty *reply
        )
{
    return xcb_get_property_value_length(reply); 
}

u32
XCBGetPropertyValueLength(
        XCBWindowProperty *reply,
        size_t size
        )
{
    return xcb_get_property_value_length(reply) / size;
}


XCBPixmap
XCBCreatePixmap(XCBDisplay *display, XCBWindow root, u16 width, u16 height, u8 depth)
{
    const XCBPixmap ret = xcb_generate_id(display);

#ifdef DBG
#endif

    xcb_create_pixmap(display, depth, ret, root, width, height);
    return ret;
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
    const u8 strlenofcursor = 6;    /* X only reads data for those 6 chars so no need for +1 for the \0 character */

    xcb_open_font(display, font, strlenofcursor, "cursor");

    const xcb_cursor_t id = xcb_generate_id(display);


#ifdef DBG
#endif

    xcb_create_glyph_cursor(display, id, font, font, shape, shape + 1,
                            fgred, fggreen, fgblue,
                            bgred, bggreen, bgblue);
    return id;
}

XCBCookie
XCBFreeCursor(XCBDisplay *display, XCBCursor cursor)
{
    XCBCookie ret = xcb_free_cursor(display, cursor);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBDefineCursor(XCBDisplay *display, XCBWindow window, XCBCursor id)
{   
    const u32 mask = XCB_CW_CURSOR;
    XCBCookie ret = xcb_change_window_attributes(display, window, mask, &id);
#ifdef DBG

#endif
    return ret;
}

XCBCookie
XCBOpenFont(XCBDisplay *display, XCBFont id, const char *name)
{
    const u16 len = strlen(name);
    XCBCookie ret = xcb_open_font(display, id, len, name);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBCloseFont(XCBDisplay *display, XCBFont id)
{
    XCBCookie ret = xcb_close_font(display, id);
    return ret;
}


/* text property textproperty */

XCBCookie
XCBGetTextPropertyCookie(XCBDisplay *display, XCBWindow window, XCBAtom property)
{
    const xcb_get_property_cookie_t cookie = xcb_get_property(display, 0, window, property, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);
    XCBCookie ret = {.sequence = cookie.sequence };

#ifdef DBG
#endif

    return ret;
}

int 
XCBGetTextPropertyReply(XCBDisplay *display, XCBCookie cookie, XCBTextProperty *reply_return)
{
    XCBGenericError *err = NULL;
    u8 status = 1;
    const xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };

    xcb_get_property_reply_t *reply = xcb_get_property_reply(display, cookie1, &err);
    
    if(!reply || reply->type == XCB_NONE)
    {
        status = 0;
    }
    else
    {
        reply_return->_reply = reply;
        reply_return->encoding = reply_return->_reply->type;
        reply_return->format = reply_return->_reply->format;
        reply_return->name_len = xcb_get_property_value_length(reply_return->_reply);
        reply_return->name = xcb_get_property_value(reply_return->_reply);
    }

    if(err)
    {   
        _xcb_err_handler(display, err);
        status = 0;
    }
    free(reply);
    return status;
}
int
XCBFreeTextProperty(XCBTextProperty *prop)
{   
#ifdef DBG
    if(!prop)
    {
        _XCB_MANUAL_DEBUG0("No prop in context.");
        XCBBreakPoint();
    }
#endif
    free(prop->_reply);
    return 1;
}

int 
XCBFlush(XCBDisplay *display)
{
    /* This locks the XServer thread (pthread mutix lock).
     * and writes the buffer to the server.
     * RETURN 1 on Success.
     * RETURN 0 on Failure.
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
XCBGetEventName(
        uint8_t response_type
        )
{
    /* cant use const cause compiler complains boo hoo */
    XCBGenericEvent ev = { .response_type = response_type };
    return XCBGetEventNameFromEvent(&ev);
}

char *
XCBGetEventNameFromEvent(
        XCBGenericEvent *event
        )
{
    const char *evs[36] = 
    {
        [XCB_NONE] = NULL,
        [XCB_KEY_PRESS] = "XCB_KEY_PRESS", 
        [XCB_KEY_RELEASE] = "XCB_KEY_RELEASE", 
        [XCB_BUTTON_PRESS] = "XCB_BUTTON_PRESS", 
        [XCB_BUTTON_RELEASE] = "XCB_BUTTON_RELEASE", 
        [XCB_MOTION_NOTIFY] = "XCB_MOTION_NOTIFY", 
        [XCB_ENTER_NOTIFY] = "XCB_ENTER_NOTIFY", 
        [XCB_LEAVE_NOTIFY] = "XCB_LEAVE_NOTIFY", 
        [XCB_FOCUS_IN] = "XCB_FOCUS_IN", 
        [XCB_FOCUS_OUT] = "XCB_FOCUS_OUT", 
        [XCB_KEYMAP_NOTIFY] = "XCB_KEYMAP_NOTIFY", 
        [XCB_EXPOSE] = "XCB_EXPOSE", 
        [XCB_GRAPHICS_EXPOSURE] = "XCB_GRAPHICS_EXPOSURE", 
        [XCB_NO_EXPOSURE] = "XCB_NO_EXPOSURE", 
        [XCB_VISIBILITY_NOTIFY] = "XCB_VISIBILITY_NOTIFY", 
        [XCB_CREATE_NOTIFY] = "XCB_CREATE_NOTIFY", 
        [XCB_DESTROY_NOTIFY] = "XCB_DESTROY_NOTIFY", 
        [XCB_UNMAP_NOTIFY] = "XCB_UNMAP_NOTIFY", 
        [XCB_MAP_NOTIFY] = "XCB_MAP_NOTIFY", 
        [XCB_MAP_REQUEST] = "XCB_MAP_REQUEST", 
        [XCB_REPARENT_NOTIFY] = "XCB_REPARENT_NOTIFY", 
        [XCB_CONFIGURE_NOTIFY] = "XCB_CONFIGURE_NOTIFY", 
        [XCB_CONFIGURE_REQUEST] = "XCB_CONFIGURE_REQUEST", 
        [XCB_GRAVITY_NOTIFY] = "XCB_GRAVITY_NOTIFY", 
        [XCB_RESIZE_REQUEST] = "XCB_RESIZE_REQUEST", 
        [XCB_CIRCULATE_NOTIFY] = "XCB_CIRCULATE_NOTIFY", 
        [XCB_CIRCULATE_REQUEST] = "XCB_CIRCULATE_REQUEST", 
        [XCB_PROPERTY_NOTIFY] = "XCB_PROPERTY_NOTIFY", 
        [XCB_SELECTION_CLEAR] = "XCB_SELECTION_CLEAR", 
        [XCB_SELECTION_REQUEST] = "XCB_SELECTION_REQUEST", 
        [XCB_SELECTION_NOTIFY] = "XCB_SELECTION_NOTIFY", 
        [XCB_COLORMAP_NOTIFY] = "XCB_COLORMAP_NOTIFY", 
        [XCB_CLIENT_MESSAGE] = "XCB_CLIENT_MESSAGE", 
        [XCB_MAPPING_NOTIFY] = "XCB_MAPPING_NOTIFY", 
        [XCB_GE_GENERIC] = "XCB_GE_GENERIC", 
    };
    if(!event)
    {   return NULL;
    }
    u8 cleanev = XCB_EVENT_RESPONSE_TYPE(event);
    /* bounds check */    /* & over && for better inlining */
    cleanev *= (cleanev > 0) & (cleanev < 35);

    return (char *)evs[cleanev];
}

char *
XCBErrorCodeText(
        uint8_t error_code)
{
    const char *errs[18] =
    {
        [0] = NULL,
        [BadRequest] = "BadRequest",
        [BadValue] = "BadValue",
        [BadWindow] = "BadWindow",
        [BadPixmap] = "BadPixmap",
        [BadAtom] = "BadAtom",
        [BadCursor] = "BadCursor",
        [BadFont] = "BadFont",
        [BadMatch] = "BadMatch",
        [BadDrawable] = "BadDrawable",
        [BadAccess] = "BadAccess",
        [BadAlloc] = "BadAlloc",
        [BadColor] = "BadColor",
        [BadGC] = "BadGC",
        [BadIDChoice] = "BadIDChoice",
        [BadName] = "BadName",
        [BadLength] = "BadLength",
        [BadImplementation] = "BadImplementation",
    };
    /* bounds check */          /* & over && for better inlining */
    error_code *= (error_code > 0) & (error_code < 18);
    return (char *)errs[error_code];
}

char *
XCBErrorMajorCodeText(
        uint8_t major_code)
{
    const char *errs[128] = 
    {
        [0] = NULL,
        [X_CreateWindow] = "CreateWindow",
        [X_ChangeWindowAttributes] = "ChangeWindowAttributes",
        [X_GetWindowAttributes] = "GetWindowAttributes",
        [X_DestroyWindow] = "DestroyWindow",
        [X_DestroySubwindows] = "DestroySubwindows",
        [X_ChangeSaveSet] = "ChangeSaveSet",
        [X_ReparentWindow] = "ReparentWindow",
        [X_MapWindow] = "MapWindow",
        [X_MapSubwindows] = "MapSubwindows",
        [X_UnmapWindow] = "UnmapWindow",
        [X_UnmapSubwindows] = "UnmapSubwindows",
        [X_ConfigureWindow] = "ConfigureWindow",
        [X_CirculateWindow] = "CirculateWindow",
        [X_GetGeometry] = "GetGeometry",
        [X_QueryTree] = "QueryTree",
        [X_InternAtom] = "InternAtom",
        [X_GetAtomName] = "GetAtomName",
        [X_ChangeProperty] = "ChangeProperty",
        [X_DeleteProperty] = "DeleteProperty",
        [X_GetProperty] = "GetProperty",
        [X_ListProperties] = "ListProperties",
        [X_SetSelectionOwner] = "SetSelectionOwner",
        [X_GetSelectionOwner] = "GetSelectionOwner",
        [X_ConvertSelection] = "ConvertSelection",
        [X_SendEvent] = "SendEvent",
        [X_GrabPointer] = "GrabPointer",
        [X_UngrabPointer] = "UngrabPointer",
        [X_GrabButton] = "GrabButton",
        [X_UngrabButton] = "UngrabButton",
        [X_ChangeActivePointerGrab] = "ChangeActivePointerGrab",
        [X_GrabKeyboard] = "GrabKeyboard",
        [X_UngrabKeyboard] = "UngrabKeyboard",
        [X_GrabKey] = "GrabKey",
        [X_UngrabKey] = "UngrabKey",
        [X_AllowEvents] = "AllowEvents",
        [X_GrabServer] = "GrabServer",
        [X_UngrabServer] = "UngrabServer",
        [X_QueryPointer] = "QueryPointer",
        [X_GetMotionEvents] = "GetMotionEvents",
        [X_TranslateCoords] = "TranslateCoords",
        [X_WarpPointer] = "WarpPointer",
        [X_SetInputFocus] = "SetInputFocus",
        [X_GetInputFocus] = "GetInputFocus",
        [X_QueryKeymap] = "QueryKeymap",
        [X_OpenFont] = "OpenFont",
        [X_CloseFont] = "CloseFont",
        [X_QueryFont] = "QueryFont",
        [X_QueryTextExtents] = "QueryTextExtents",
        [X_ListFonts] = "ListFonts",
        [X_ListFontsWithInfo] = "ListFontsWithInfo",
        [X_SetFontPath] = "SetFontPath",
        [X_GetFontPath] = "GetFontPath",
        [X_CreatePixmap] = "CreatePixmap",
        [X_FreePixmap] = "FreePixmap",
        [X_CreateGC] = "CreateGC",
        [X_ChangeGC] = "ChangeGC",
        [X_CopyGC] = "CopyGC",
        [X_SetDashes] = "SetDashes",
        [X_SetClipRectangles] = "SetClipRectangles",
        [X_FreeGC] = "FreeGC",
        [X_ClearArea] = "ClearArea",
        [X_CopyArea] = "CopyArea",
        [X_CopyPlane] = "CopyPlane",
        [X_PolyPoint] = "PolyPoint",
        [X_PolyLine] = "PolyLine",
        [X_PolySegment] = "PolySegment",
        [X_PolyRectangle] = "PolyRectangle",
        [X_PolyArc] = "PolyArc",
        [X_FillPoly] = "FillPoly",
        [X_PolyFillRectangle] = "PolyFillRectangle",
        [X_PolyFillArc] = "PolyFillArc",
        [X_PutImage] = "PutImage",
        [X_GetImage] = "GetImage",
        [X_PolyText8] = "PolyText8",
        [X_PolyText16] = "PolyText16",
        [X_ImageText8] = "ImageText8",
        [X_ImageText16] = "ImageText16",
        [X_CreateColormap] = "CreateColormap",
        [X_FreeColormap] = "FreeColormap",
        [X_CopyColormapAndFree] = "CopyColormapAndFree",
        [X_InstallColormap] = "InstallColormap",
        [X_UninstallColormap] = "UninstallColormap",
        [X_ListInstalledColormaps] = "ListInstalledColormaps",
        [X_AllocColor] = "AllocColor",
        [X_AllocNamedColor] = "AllocNamedColor",
        [X_AllocColorCells] = "AllocColorCells",
        [X_AllocColorPlanes] = "AllocColorPlanes",
        [X_FreeColors] = "FreeColors",
        [X_StoreColors] = "StoreColors",
        [X_StoreNamedColor] = "StoreNamedColor",
        [X_QueryColors] = "QueryColors",
        [X_LookupColor] = "LookupColor",
        [X_CreateCursor] = "CreateCursor",
        [X_CreateGlyphCursor] = "CreateGlyphCursor",
        [X_FreeCursor] = "FreeCursor",
        [X_RecolorCursor] = "RecolorCursor",
        [X_QueryBestSize] = "QueryBestSize",
        [X_QueryExtension] = "QueryExtension",
        [X_ListExtensions] = "ListExtensions",
        [X_ChangeKeyboardMapping] = "ChangeKeyboardMapping",
        [X_GetKeyboardMapping] = "GetKeyboardMapping",
        [X_ChangeKeyboardControl] = "ChangeKeyboardControl",
        [X_GetKeyboardControl] = "GetKeyboardControl",
        [X_Bell] = "Bell",
        [X_ChangePointerControl] = "ChangePointerControl",
        [X_GetPointerControl] = "GetPointerControl",
        [X_SetScreenSaver] = "SetScreenSaver",
        [X_GetScreenSaver] = "GetScreenSaver",
        [X_ChangeHosts] = "ChangeHosts",
        [X_ListHosts] = "ListHosts",
        [X_SetAccessControl] = "SetAccessControl",
        [X_SetCloseDownMode] = "SetCloseDownMode",
        [X_KillClient] = "KillClient",
        [X_RotateProperties] = "RotateProperties",
        [X_ForceScreenSaver] = "ForceScreenSaver",
        [X_SetPointerMapping] = "SetPointerMapping",
        [X_GetPointerMapping] = "GetPointerMapping",
        [X_SetModifierMapping] = "SetModifierMapping",
        [X_GetModifierMapping] = "GetModifierMapping",
        [X_NoOperation] = "NoOperation"
    };
    /* bounds check */          /* & over && for better inlining */
    major_code *= (major_code > 0) & (major_code < 128);
    return (char *)errs[major_code];
}

char *
XCBErrorMinorCodeText(
        uint16_t minor_code
        )
{   
    return NULL;
}

char *
XCBErrorDisplayText(
        uint8_t display_error
        )
{
    char *errs[7] = 
    {
        [0] = NULL,
        [XCB_CONN_ERROR] = "DisplayError",
        [XCB_CONN_CLOSED_EXT_NOTSUPPORTED] = "ExtensionNotSupported",
        [XCB_CONN_CLOSED_MEM_INSUFFICIENT] = "OutOfMemory",
        [XCB_CONN_CLOSED_REQ_LEN_EXCEED] = "TooManyRequests",
        [XCB_CONN_CLOSED_PARSE_ERR] = "InvalidDisplay",
        [XCB_CONN_CLOSED_INVALID_SCREEN] = "InvalidScreen"
    };
    /* bounds check */              /* & over && for better inlining */
    display_error *= (display_error > 0) & (display_error < 7);
    return errs[display_error];
}



/* events */



XCBCookie
XCBAllowEvents(XCBDisplay *display, u8 mode, XCBTimestamp tim)
{
    XCBCookie ret = xcb_allow_events(display, mode, tim);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBSendEvent(
        XCBDisplay *display,
        XCBWindow window,
        uint8_t propagate,
        uint32_t event_mask,
        const char *event
        )
{
    XCBCookie ret = xcb_send_event(display, propagate, window, event_mask, event);
#ifdef DBG
#endif
    return ret;
}

int 
XCBNextEvent(XCBDisplay *display, XCBGenericEvent **event_return) 
{
    /* waits till next event happens before returning */
    return !!((*event_return = xcb_wait_for_event(display)));
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

void *
XCBCheckReply(
        XCBDisplay *display, 
        XCBCookie request)
{
    XCBGenericError *err = NULL;
    void *reply = NULL;
    xcb_poll_for_reply(display, request.sequence, &reply, &err);

    if(err)
    {    
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

void *
XCBCheckReply64(
        XCBDisplay *display, 
        XCBCookie64 request)
{   
    XCBGenericError *err = NULL;
    void *reply = NULL;
    xcb_poll_for_reply64(display, request.sequence, &reply, &err);

    if(err)
    {    
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}


void *
XCBWaitForReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    void *reply = xcb_wait_for_reply(display, cookie.sequence, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

void *
XCBWaitForReply64(
        XCBDisplay *display,
        XCBCookie64 cookie
        )
{
    XCBGenericError *err = NULL;
    void *reply = xcb_wait_for_reply64(display, cookie.sequence, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

XCBCookie
XCBGrabKeyboardCookie(
        XCBDisplay *display,
        XCBWindow grab_window,
        uint8_t owner_events,
        uint8_t pointer_mode,
        uint8_t keyboard_mode,
        XCBTimestamp tim
        )
{
    const xcb_grab_keyboard_cookie_t cookie = xcb_grab_keyboard(display, owner_events, grab_window, tim, pointer_mode, keyboard_mode);
    return (XCBCookie) {.sequence = cookie.sequence };
}

XCBGrabKeyboard *
XCBGrabKeyboardReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    const xcb_grab_keyboard_cookie_t cookie1 = { .sequence = cookie.sequence };
    XCBGrabKeyboard *reply = xcb_grab_keyboard_reply(display, cookie1, &err);

    if(err)
    {
        _xcb_err_handler(display, err);
        free(reply);
        return NULL;
    }
    return reply;
}

XCBCookie
XCBUngrabKeyboard(
        XCBDisplay *display,
        XCBTimestamp tim
        )
{
    XCBCookie ret = xcb_ungrab_keyboard(display, tim);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBGrabKey(XCBDisplay *display, XCBKeyCode keycode, u16 modifiers, XCBWindow grab_window, u8 owner_events, u8 pointer_mode, u8 keyboard_mode)
{
    XCBCookie ret = xcb_grab_key(display, owner_events, grab_window, modifiers, keycode, pointer_mode, keyboard_mode);
#ifdef DBG
#endif

    return ret;
}

XCBCookie 
XCBUngrabKey(XCBDisplay *display, XCBKeyCode key, u16 modifiers, XCBWindow grab_window)
{
    XCBCookie ret = xcb_ungrab_key(display, key, grab_window, modifiers);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBUngrabButton(XCBDisplay *display, uint8_t button, uint16_t modifier, XCBWindow window)
{
    XCBCookie ret = xcb_ungrab_button(display, button, window, modifier);
#ifdef DBG
#endif

    return ret;
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
    XCBCookie ret = xcb_grab_button(display, owner_events, grab_window, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, button, modifiers);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBGrabPointerCookie(
        XCBDisplay *display,
        XCBWindow grab_window,
        uint8_t owner_events,
        uint16_t event_mask,
        uint8_t pointer_mode,
        uint8_t keyboard_mode,
        XCBWindow confine_to,
        XCBCursor cursor,
        XCBTimestamp tim
        )
{
    xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer(
            display, owner_events, grab_window, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, tim
            );
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;

}

XCBGrabPointer *
XCBGrabPointerReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    const xcb_grab_pointer_cookie_t cookie1 = { .sequence = cookie.sequence };
    XCBGenericError *err = NULL;
    xcb_grab_pointer_reply_t *reply = xcb_grab_pointer_reply(display, cookie1, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;

}


int
XCBDisplayKeyCodes(XCBDisplay *display, int *min_keycode_return, int *max_keycode_return)
{
    const XCBSetup *setup = xcb_get_setup(display);
    *min_keycode_return = setup->min_keycode;
    *max_keycode_return = setup->max_keycode;
    /* xcb source: "length, invalid value" so if this is ever 0 we "failed" */
    return !!setup->length;
}

int 
XCBDisplayKeycodes(XCBDisplay *display, int *min_keycode_return, int *max_keycode_return)
{
    const XCBSetup *setup = xcb_get_setup(display);
    *min_keycode_return = setup->min_keycode;
    *max_keycode_return = setup->max_keycode;
    /* xcb source: "length, invalid value" so if this is ever 0 we "failed" */
    return !!setup->length;
}

XCBKeyCode *
XCBGetKeyCodes(XCBDisplay *display, XCBKeysym keysym)
{
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

uint8_t
XCBRefreshKeyboardMapping(
        XCBKeySymbols *syms, 
        XCBMappingNotifyEvent *event)
{
    return xcb_refresh_keyboard_mapping(syms, event);
}

XCBCookie
XCBGetKeyboardMappingCookie(XCBDisplay *display, XCBKeyCode first_keycode, u8 count)
{
    const xcb_get_keyboard_mapping_cookie_t cookie = xcb_get_keyboard_mapping(display, first_keycode, count);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBKeyboardMapping *
XCBGetKeyboardMappingReply(XCBDisplay *display, XCBCookie cookie)
{
    XCBGenericError *err = NULL;
    const xcb_get_keyboard_mapping_cookie_t cookie1 = { .sequence = cookie.sequence };
    XCBKeyboardMapping *reply = xcb_get_keyboard_mapping_reply(display, cookie1, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

XCBCookie
XCBQueryTreeCookie(
        XCBDisplay *display,
        XCBWindow window
        )
{
    const xcb_query_tree_cookie_t cookie = xcb_query_tree(display, window);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBQueryTree *
XCBQueryTreeReply(XCBDisplay *display, XCBCookie cookie)
{
    const xcb_query_tree_cookie_t cookie1 = { .sequence = cookie.sequence };
    XCBGenericError *err = NULL;
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(display, cookie1, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}

XCBWindow *
XCBQueryTreeChildren(const XCBQueryTree *tree)
{
    /* Why does this exist? its simply a offset of (tree + 1) */
    return xcb_query_tree_children(tree);
}


XCBCookie
XCBQueryPointerCookie(XCBDisplay *display, XCBWindow window)
{
    const xcb_query_pointer_cookie_t cookie = xcb_query_pointer(display, window);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBQueryPointer *
XCBQueryPointerReply(XCBDisplay *display, XCBCookie cookie)
{
    XCBGenericError *err = NULL;
    const xcb_query_pointer_cookie_t cookie1 = { .sequence = cookie.sequence };
    XCBQueryPointer *reply = xcb_query_pointer_reply(display, cookie1, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        if(reply)
        {   free(reply);
        }
        return NULL;
    }
    return reply;
}


XCBCookie
XCBMapWindow(XCBDisplay *display, XCBWindow window)
{
    XCBCookie ret = xcb_map_window(display, window);
#ifdef DBG
#endif
    return ret;
}


XCBCookie
XCBUnmapWindow(
        XCBDisplay *display,
        XCBWindow window
        )
{
    XCBCookie ret = xcb_unmap_window(display, window);
#ifdef DBG
#endif
    return ret;
}


XCBCookie
XCBDestroyWindow(
        XCBDisplay *display,
        XCBWindow window
        )
{
    XCBCookie ret = xcb_destroy_window(display, window);
#ifdef DBG
#endif
    return ret;
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

    /* not actually used but just for standards */
    XCBCookie ret = xcb_create_window(display, depth, id, parent, x, y, width, height, border_width, 
    class, visual, valuemask, used);
    
#ifdef DBG
#endif

    (void)ret;
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
    const u8 depth = XCB_COPY_FROM_PARENT;
    const XCBVisual visual = XCBGetScreen(display)->root_visual;
    const u8  class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    const u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL;
    const u32 color[2] = { background_color, border_color };

    /* not actually used but just for standards */
    XCBCookie ret = xcb_create_window(display, depth, id, parent, x, y, width, height, border_width, class, visual, mask, &color);
#ifdef DBG
#endif
    (void)ret;
    return id;
}


XCBGC
XCBCreateGC(XCBDisplay *display, XCBDrawable drawable, 
u32 valuemask, const void *valuelist)
{
    const XCBGC id = xcb_generate_id(display);

    /* not actually used but just for standards */
    XCBCookie ret = xcb_create_gc(display, id, drawable, valuemask, valuelist);
#ifdef DBG
#endif

    (void)ret;
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


    XCBCookie ret = xcb_change_gc(display, gc, mask, gcvalist);
#ifdef DBG
#endif

    /* This returns a cookie but changing the gc isnt doesnt really require a reply as you are directly manupulating your own gc
     */
    /* X11 src/SetLStyle.c always returns 1 (for some reason) */
    return !!ret.sequence;
}

XCBCookie
XCBChangeProperty(XCBDisplay *display, XCBWindow window, XCBAtom property, XCBAtom type, u8 format, u8 mode, const void *data, u32 nelements)
{
    XCBCookie ret = xcb_change_property(display, mode, window, property, type, format, nelements, data);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBDeleteProperty(XCBDisplay *display, XCBWindow window, XCBAtom property)
{
    XCBCookie ret = xcb_delete_property(display, window, property);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBConfigureWindow(
        XCBDisplay *display, 
        XCBWindow window,
        u16 value_mask,
        XCBWindowChanges *changes)
{
    XCBCookie ret = xcb_configure_window_aux(display, window, value_mask, changes);
#ifdef DBG
#endif

    return ret;
}

XCBCookie
XCBSetClassHint(XCBDisplay *display, XCBWindow window, const char *class_name)
{
    const int len = strlen(class_name);
    XCBCookie ret = xcb_change_property(display, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, len, class_name);
#ifdef DBG
#endif

    return ret;
}


XCBCookie
XCBChangeGC(XCBDisplay *display, XCBGC gc, u32 valuemask, const void *valuelist)
{
    XCBCookie ret = xcb_change_gc(display, gc, valuemask, valuelist);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBDrawPoint(XCBDisplay *display, u8 coordinatemode, XCBDrawable drawable, XCBGC gc, uint32_t points_len, XCBPoint *points)
{
    XCBCookie ret = xcb_poly_point(display, coordinatemode, drawable, gc, points_len, points);
#ifdef DBG
#endif

    return ret;
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

XCBCookie64
XCBWiden(XCBDisplay *display, XCBCookie cookie)
{
    /* this might break in the future so we dont want to typedef XCBDisplay */
    struct spoofIn
    {
        pthread_cond_t event_cond;
        int reading;
        char queue[4096];
        int queue_len;

        uint64_t request_expected;
        uint64_t request_read;
        uint64_t request_completed;
        uint64_t total_read;
        struct reply_list *current_reply;
        struct reply_list **current_reply_tail;

        void *replies;
        struct event_list *events;
        struct event_list **events_tail;
        struct reader_list *readers;
        struct special_list *special_waiters;

        struct pending_reply *pending_replies;
        struct pending_reply **pending_replies_tail;
    #if HAVE_SENDMSG
        _xcb_fd in_fd;
    #endif
        struct xcb_special_event *special_events;
    };
    enum lazy_reply_tag
    {
        LAZY_NONE = 0,
        LAZY_COOKIE,
        LAZY_FORCED
    };
    struct spoofOut {
        pthread_cond_t cond;
        int writing;

        pthread_cond_t socket_cond;
        void (*return_socket)(void *closure);
        void *socket_closure;
        int socket_moving;

        char queue[16384];
        int queue_len;

        uint64_t request;
        uint64_t request_written;
        uint64_t request_expected_written;
        uint64_t total_written;

        pthread_mutex_t reqlenlock;
        enum lazy_reply_tag maximum_request_length_tag;
        union {
            unsigned int cookie;
            uint32_t value;
        } maximum_request_length;
        #if HAVE_SENDMSG
        _xcb_fd out_fd;
        #endif
    };
    struct spoofDisplay
    {
        int has_error;
        xcb_setup_t *setup;
        int fd;
        pthread_mutex_t iolock;
        struct spoofIn in;
        struct spoofOut out;
        uint64_t infinity[1024];
    };
    struct spoofDisplay *disp = (struct spoofDisplay *)display;
    u64 widen = (disp->out.request & UINT64_C(0xffffffff00000000)) | cookie.sequence;
    if(widen > disp->out.request)
    {   widen -= UINT64_C(1) << 32;
    }
    return (XCBCookie64){ .sequence = widen };
}

XCBCookie64 
XCBWidenCookie(XCBDisplay *display, XCBCookie cookie)
{
    return XCBWiden(display, cookie);
}

/* ICCCM */

XCBCookie
XCBGetWMProtocolsCookie(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBAtom protocol)
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_protocols(display, window, protocol);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

int
XCBGetWMProtocolsReply(
        XCBDisplay *display, 
        XCBCookie cookie,
        XCBWMProtocols *protocol_return
        )
{  
    XCBGenericError *err = NULL;
    const xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };
    const int ret = xcb_icccm_get_wm_protocols_reply(display, cookie1, protocol_return, &err);
    if(err)
    {   
        _xcb_err_handler(display, err);
        return 0;
    }
    return ret;
}

void
XCBWipeGetWMProtocols(
        XCBWMProtocols *protocols)
{
    xcb_icccm_get_wm_protocols_reply_wipe(protocols);
}


XCBCookie
XCBGetWMHintsCookie(
        XCBDisplay *display,
        XCBWindow win
        )
{ 
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints(display, win);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

XCBWMHints *
XCBGetWMHintsReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };

    xcb_get_property_reply_t *reply = xcb_get_property_reply(display, cookie1, &err);
    XCBWMHints *data = NULL;

    /* error handling */
    if(err)
    {   
        _xcb_err_handler(display, err);
        goto FAILURE;
    }
    /* make sure data matches */
    if(!reply || reply->type != XCB_ATOM_WM_HINTS || reply->format != 32)
    {   goto FAILURE;   
    }

    data = xcb_get_property_value(reply);
    if(!data)
    {   goto FAILURE;
    }

    u32 length = xcb_get_property_value_length(reply);
    u32 num_elem = length / (reply->format / 8);
    
    if(num_elem < XCB_ICCCM_NUM_WM_HINTS_ELEMENTS - 1)
    {   goto FAILURE;
    }

    if(num_elem < XCB_ICCCM_NUM_WM_HINTS_ELEMENTS)
    {   data->window_group = XCB_NONE;
    }

    /* yes this this is xcb_size_hints_t no its not a mistake */
    if(length > sizeof(xcb_size_hints_t))
    {   length = sizeof(xcb_size_hints_t);
    }

    /* yes this this is xcb_size_hints_t no its not a mistake */
    xcb_size_hints_t safedata;


    memcpy(&safedata, (xcb_size_hints_t *)data, length);
    memcpy(reply, (xcb_size_hints_t *)&safedata, length);

    return (XCBWMHints *)reply;
FAILURE:
    free(reply);
    return NULL;
}

XCBCookie
XCBSetWMHintsCookie(
        XCBDisplay *display,
        XCBWindow window,
        XCBWMHints *wmhints
        )
{
    XCBCookie ret = xcb_icccm_set_wm_hints(display, window, wmhints);
#ifdef DBG
#endif
    return ret;
}

XCBCookie
XCBGetWMNormalHintsCookie(
        XCBDisplay *display,
        XCBWindow win
        )
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_normal_hints(display, win);
    XCBCookie ret = { .sequence = cookie.sequence };
#ifdef DBG
#endif
    return ret;
}

uint8_t
XCBGetWMNormalHintsReply(
        XCBDisplay *display,
        XCBCookie cookie,
        XCBSizeHints *hints_return
        )
{
    XCBGenericError *err = NULL;
    xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };
    u8 status = xcb_icccm_get_wm_normal_hints_reply(display, cookie1, hints_return, &err);

    if(err)
    {   _xcb_err_handler(display, err);
        status = 0;
    }
    return status;
}

XCBCookie
XCBGetWMClassCookie(
        XCBDisplay *display, 
        XCBWindow win
        )
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class(display, win);
    XCBCookie ret = { .sequence = cookie.sequence };

#ifdef DBG
#endif

    return ret;
}

u8
XCBGetWMClassReply(
        XCBDisplay *display,
        XCBCookie cookie,
        XCBWMClass *class_return
        )
{
    XCBGenericError *err = NULL;
    xcb_get_property_cookie_t cookie1 = { .sequence = cookie.sequence };
    u8 status = xcb_icccm_get_wm_class_reply(display, cookie1, class_return, &err);
    
    if(err)
    {
        _xcb_err_handler(display, err);
        status = 0;
    }

    return status;
}

void
XCBWipeGetWMClass(XCBWMClass *class)
{
#ifdef DBG
    if(!class)
    {   _XCB_MANUAL_DEBUG0("No class to wipe? this shouldnt be possible.");
        XCBBreakPoint();
    }
#endif
    xcb_icccm_get_wm_class_reply_wipe(class);
}





