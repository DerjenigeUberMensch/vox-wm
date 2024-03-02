/**
 * @file xcb_trl.h
 * Code definitions.
 */

/* compiling
 * xcb is dumb and sometimes doesnt find the required stuff so you just guess or search it up
 * but this shhould cover most if not all of xcb's libraries, atleast the ones used here
 * `pkg-config --cflags --libs xcb` -lxcb-util -lxcb-icccm -lxcb-keysyms
 */


/* Why use this?
 * you shouldnt, this was mostly for self-documentation and for macros.
 * Why make this?
 * xcb snake_case is a dumb change (in my opinion), and should have stayed camelCase or PascalCase.
 * but it likely wasnt to "dissasociate" from Xlib which is fine, however I personally dont like that form of casing.
 * also some changes encourage a certain type of usage which is fine, however when "some things" are poorly documented.
 * It becomes quite hard to understand those design choices.
 * However for the most part the important things are documented.
 * Secondly the design choices really make xcb more like a Async Xlib which is strange.
 * Is this bloat?
 * Yes, yes it is.
 * Why not seperate files?
 * 1. Faster compiling.
 * 2. ctrl+f or '/' for search, no need for grep.
 * 3. too much work.
 * 4. I am Lazy.
 */

/*
 * Basic XCB Usage
 *
 * Cookies;
 *
 * Cookies are simply structures with ids.
 * These cookies are mostly useless and just ids for xcb to use.
 *
 * However you can do 2 unique things with them.
 *
 * Self Spoofing;
 *
 * Events in xcb are quite different compared to Xlib and are much simpler in theory.
 * Due to this we can self Spoof calls to our current event handler.
 * This is sometimes useful when you dont want to receive an event back (for whatever reason).
 * Or when you want to receive events back from yourself which can be useful most of the time.
 *
 * # This would send an event to the XServer that we want to map this window as a mapnotify (assuming we sent it).
 * # and have to be handle by us.
 *
 * XCBMapWindow(dpy, cool_window);
 *
 * # <<< Event Generated >>> #
 * 
 *
 * However we can "spoof" no reply and generate no event for ourselves.
 *
 * XCBCookie cookie;
 * cookie = XCBMapWindow(dpy, cool_window);
 * XCBDiscardReply(dpy, cookie);
 *
 * # <<< No Event Generated >>> #
 *
 * This may not "always" work due to how xcb works so, we must to call XCBDiscardReply() after our generated cookie.
 * 
 * XCBCookie cookie;
 * cookie = XCBMapWindow(dpy, cool_window);
 * # Sync the Display
 * XCBSync(dpy) # or XCBFlush(dpy);
 *
 * # This does nothing because we flushed the buffer to the display using XCBSync()
 * XCBDiscardReply(dpy, cookie);
 *
 * # Reply64
 * # This really isnt too important and is mostly for 64 bit platforms
 * XCBDiscardReply64(dpy, cookie);
 *
 *
 * Reply backs;
 *
 * The second thing we can do is ask for a reply back from the XServer.
 * This can be done manually using _XReply(), _XRead(), _XWrite() however xcb abstracts that away.
 * Because of that we can use xcb_cool_function_ and then _reply() so xcb_cool_function_reply(XCBDisplay *display, XCBCookie cookie);
 * Some xcb functions dont have a _reply() version and those are simply handled in the event queue;
 * 
 * Errors;
 *
 * Depending on the type of error a few things can occur.
 *
 * <<< XCBConnection Error >>> ;
 * This type of error can happen out of the blue if you arent careful.
 * To check if this error occured simply acces the display->has_error type;
 * If you want a more clear approach you can simply call XCBHasConnectionError();
 *
 * <<< Reply Errors >>> ;
 * By default Errors are handled in the event que.
 * However reply backs, (ie any function that calls for a reply from a cookie).
 * Instead uses the XCBErrorHandler(), this can be set by the client using XCBSetErrorHandler().
 * By Default however is to simply call die() on errors, which may not be desirable.
 */

#ifndef XCB_PTL_TYPEDEF_H_
#define XCB_PTL_TYPEDEF_H_

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_event.h>
#include <xcb/xcbext.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xinerama.h>
#include <xcb/xcb_xrm.h>


/* Ghosts */

/*  FILE xcb_conn.c ;
 *  PUBLIC: ;
 *  int has_error -> This is Nonzero on error ;
 *  XCBSetup *setup -> This is a struct pointer to the main display properties: {root, width, height...} ;
 *  int fd -> This is short for file descriptor (AKA connection number) ;
 *  PRIVATE: ;
 *  pthread_mutex_t iolock -> This should be considered READONLY ;
 *  _xcb_in in -> This should be considered READONLY ;
 *  _xcb_out out -> This should be considered READONLY ;
 *  _xcb_ext ext -> This should be considered READONLY ;
 *  _xcb_xid xid -> This should be considered READONLY ;
 */
typedef xcb_connection_t XCBConnection;
/*  FILE: src/xcb_conn.c ;
 *  PUBLIC: ;
 *  int has_error -> This is Nonzero on error ;
 *  XCBSetup *setup -> This is a struct pointer to the main display properties: {root, width, height...} ;
 *  int fd -> This is short for file descriptor (AKA connection number) ;
 *  PRIVATE: ;
 *  pthread_mutex_t iolock -> This should be considered READONLY ;
 *  _xcb_in in -> This should be considered READONLY ;
 *  _xcb_out out -> This should be considered READONLY ;
 *  _xcb_ext ext -> This should be considered READONLY ;
 *  _xcb_xid xid -> This should be considered READONLY ;
 */
typedef xcb_connection_t XCBDisplay;
/* FILE: xcb/xcb.h ;
 * PUBLIC: ;
 * u8 status -> ;
 * u16 protocol_major_version -> ;
 * u16 protocol_minor_version -> ;
 * u16 length -> ;
 * u32 release_number -> ;
 * u32 resource_id_base -> ;
 * u32 resource_id_mask -> ;
 * u32 motion_buffer_size -> ;
 * u16 vendor_len -> ;
 * u16 maximum_request_length -> ;
 * u8  roots_len -> ;
 * u8  pixmap_formats_len -> ;
 * u8  image_byte_order -> ;
 * u8  bitmap_format_bit_order -> ;
 * u8  bitmap_format_scanline_unit -> ;
 * u8  bitmap_format_scanline_pad -> ;
 * XCBKeyCode min_keycode -> ;
 * XCBKeyCode max_keycode -> ;
 * PRIVATE: ;
 * u8 pad0 -> This should be considered READONLY ;
 * u8 pad1[4] -> This should be considered READONLY ;
 */
typedef xcb_setup_t XCBSetup;
typedef xcb_key_symbols_t XCBKeySymbols;
typedef xcb_screen_iterator_t XCBScreenIterator;
typedef xcb_screen_t XCBScreen;
typedef xcb_window_t XCBWindow;
typedef xcb_cursor_t XCBCursor;
typedef xcb_get_property_cookie_t XCBTextPropertyCookie;
typedef xcb_icccm_get_text_property_reply_t XCBTextProperty;
typedef xcb_void_cookie_t XCBCookie;
typedef xcb_get_keyboard_mapping_cookie_t XCBKeyboardMappingCookie;
typedef xcb_get_keyboard_mapping_reply_t XCBKeyboardMapping;
typedef xcb_get_modifier_mapping_cookie_t XCBKeyboardModifierCookie;
typedef xcb_get_modifier_mapping_reply_t XCBKeyboardModifier;
typedef xcb_intern_atom_cookie_t XCBAtomCookie;
typedef xcb_atom_t XCBAtom;

/* Analagous to Xlib's XA_(type)
 * XCB_ATOM_NONE = 0,
 * XCB_ATOM_ANY = 0,
 * XCB_ATOM_PRIMARY = 1,
 * XCB_ATOM_SECONDARY = 2,
 * XCB_ATOM_ARC = 3,
 * XCB_ATOM_ATOM = 4,
 * XCB_ATOM_BITMAP = 5,
 * XCB_ATOM_CARDINAL = 6,
 * XCB_ATOM_COLORMAP = 7,
 * XCB_ATOM_CURSOR = 8,
 * XCB_ATOM_CUT_BUFFER0 = 9,
 * XCB_ATOM_CUT_BUFFER1 = 10,
 * XCB_ATOM_CUT_BUFFER2 = 11,
 * XCB_ATOM_CUT_BUFFER3 = 12,
 * XCB_ATOM_CUT_BUFFER4 = 13,
 * XCB_ATOM_CUT_BUFFER5 = 14,
 * XCB_ATOM_CUT_BUFFER6 = 15,
 * XCB_ATOM_CUT_BUFFER7 = 16,
 * XCB_ATOM_DRAWABLE = 17,
 * XCB_ATOM_FONT = 18,
 * XCB_ATOM_INTEGER = 19,
 * XCB_ATOM_PIXMAP = 20,
 * XCB_ATOM_POINT = 21,
 * XCB_ATOM_RECTANGLE = 22,
 * XCB_ATOM_RESOURCE_MANAGER = 23,
 * XCB_ATOM_RGB_COLOR_MAP = 24,
 * XCB_ATOM_RGB_BEST_MAP = 25,
 * XCB_ATOM_RGB_BLUE_MAP = 26,
 * XCB_ATOM_RGB_DEFAULT_MAP = 27,
 * XCB_ATOM_RGB_GRAY_MAP = 28,
 * XCB_ATOM_RGB_GREEN_MAP = 29,
 * XCB_ATOM_RGB_RED_MAP = 30,
 * XCB_ATOM_STRING = 31,
 * XCB_ATOM_VISUALID = 32,
 * XCB_ATOM_WINDOW = 33,
 * XCB_ATOM_WM_COMMAND = 34,
 * XCB_ATOM_WM_HINTS = 35,
 * XCB_ATOM_WM_CLIENT_MACHINE = 36,
 * XCB_ATOM_WM_ICON_NAME = 37,
 * XCB_ATOM_WM_ICON_SIZE = 38,
 * XCB_ATOM_WM_NAME = 39,
 * XCB_ATOM_WM_NORMAL_HINTS = 40,
 * XCB_ATOM_WM_SIZE_HINTS = 41,
 * XCB_ATOM_WM_ZOOM_HINTS = 42,
 * XCB_ATOM_MIN_SPACE = 43,
 * XCB_ATOM_NORM_SPACE = 44,
 * XCB_ATOM_MAX_SPACE = 45,
 * XCB_ATOM_END_SPACE = 46,
 * XCB_ATOM_SUPERSCRIPT_X = 47,
 * XCB_ATOM_SUPERSCRIPT_Y = 48,
 * XCB_ATOM_SUBSCRIPT_X = 49,
 * XCB_ATOM_SUBSCRIPT_Y = 50,
 * XCB_ATOM_UNDERLINE_POSITION = 51,
 * XCB_ATOM_UNDERLINE_THICKNESS = 52,
 * XCB_ATOM_STRIKEOUT_ASCENT = 53,
 * XCB_ATOM_STRIKEOUT_DESCENT = 54,
 * XCB_ATOM_ITALIC_ANGLE = 55,
 * XCB_ATOM_X_HEIGHT = 56,
 * XCB_ATOM_QUAD_WIDTH = 57,
 * XCB_ATOM_WEIGHT = 58,
 * XCB_ATOM_POINT_SIZE = 59,
 * XCB_ATOM_RESOLUTION = 60,
 * XCB_ATOM_COPYRIGHT = 61,
 * XCB_ATOM_NOTICE = 62,
 * XCB_ATOM_FONT_NAME = 63,
 * XCB_ATOM_FAMILY_NAME = 64,
 * XCB_ATOM_FULL_NAME = 65,
 * XCB_ATOM_CAP_HEIGHT = 66,
*/
typedef xcb_atom_enum_t XCBAtomType;
typedef xcb_keysym_t XCBKeysym;
typedef xcb_keycode_t XCBKeycode;
typedef xcb_keycode_t XCBKeyCode;

typedef xcb_visualid_t XCBVisual;


/* array of characters NEVER use as char */
typedef char XCBClassHint;


typedef xcb_get_window_attributes_cookie_t XCBWindowAttributesCookie;
typedef xcb_configure_window_value_list_t XCBWindowChanges;
typedef xcb_get_window_attributes_cookie_t XCBAttributesCookie;
typedef xcb_get_window_attributes_cookie_t XCBGetAttributesCookie;
typedef xcb_get_geometry_cookie_t XCBGetGeometryCookie;
typedef xcb_get_geometry_cookie_t XCBGeometryCookie;
typedef xcb_get_geometry_cookie_t XCBWindowGeometryCookie;
typedef xcb_query_extension_cookie_t XCBExtensionCookie;
typedef xcb_query_extension_reply_t XCBExtensionReply;
typedef xcb_change_window_attributes_value_list_t XCBWindowAttributes;
typedef xcb_get_window_attributes_reply_t XCBWindowAttributesReply;
typedef xcb_get_geometry_reply_t XCBGeometry;
typedef xcb_get_geometry_reply_t XCBWindowGeometry;
typedef xcb_pixmap_t XCBPixmap;
typedef xcb_gcontext_t XCBGC;
typedef xcb_drawable_t XCBDrawable;
typedef xcb_point_t XCBPoint;
typedef xcb_font_t XCBFont;
typedef xcb_generic_event_t XCBGenericEvent;
typedef xcb_generic_reply_t XCBGenericReply;
typedef xcb_generic_error_t XCBGenericError;

typedef xcb_query_pointer_cookie_t XCBPointerCookie;
typedef xcb_query_pointer_reply_t XCBPointerReply;

/* Xinerama */

typedef xcb_xinerama_screen_info_t XCBXineramaScreenInfo;
typedef xcb_xinerama_is_active_cookie_t XCBXineramaIsActiveCookie;
typedef xcb_xinerama_is_active_reply_t XCBXineramaIsActiveReply;

typedef xcb_xinerama_query_screens_reply_t XCBXineramaQueryScreensReply;
typedef xcb_xinerama_query_screens_cookie_t XCBXineramaQueryScreensCookie;



/* events */
typedef xcb_key_press_event_t XCBKeyPressEvent;
typedef xcb_key_release_event_t XCBKeyReleaseEvent;
typedef xcb_button_press_event_t XCBButtonPressEvent;
typedef xcb_button_release_event_t XCBButtonReleaseEvent;
typedef xcb_motion_notify_event_t XCBMotionNotifyEvent;
typedef xcb_enter_notify_event_t XCBEnterNotifyEvent;
typedef xcb_leave_notify_event_t XCBLeaveNotifyEvent;
typedef xcb_focus_in_event_t XCBFocusInEvent;
typedef xcb_focus_out_event_t XCBFocusOutEvent;
typedef xcb_keymap_notify_event_t XCBKeymapNotifyEvent;
typedef xcb_expose_event_t XCBExposeEvent;
typedef xcb_graphics_exposure_event_t XCBGraphicsExposeEvent;
typedef xcb_graphics_exposure_event_t XCBGraphicsExposureEvent;
typedef xcb_no_exposure_event_t XCBNoExpose;
typedef xcb_no_exposure_event_t XCBNoExposure;
typedef xcb_circulate_notify_event_t XCBCirculateNotifyEvent;
typedef xcb_circulate_request_event_t XCBCirculateRequestEvent;
typedef xcb_map_request_event_t XCBMapRequestEvent;
typedef xcb_configure_request_event_t XCBConfigureRequestEvent;
typedef xcb_configure_notify_event_t XCBConfigureNotifyEvent;
typedef xcb_resize_request_event_t XCBResizeRequestEvent;
typedef xcb_create_notify_event_t XCBCreateNotifyEvent;
typedef xcb_destroy_notify_event_t XCBDestroyNotifyEvent;
typedef xcb_gravity_notify_event_t XCBGravityNotifyEvent;
typedef xcb_map_notify_event_t XCBMapNotifyEvent;
typedef xcb_mapping_notify_event_t XCBMappingNotifyEvent;
typedef xcb_unmap_notify_event_t XCBUnMapNotifyEvent;
typedef xcb_visibility_notify_event_t XCBVisibilityNotifyEvent;
typedef xcb_reparent_notify_event_t XCBReparentNotifyEvent;
typedef xcb_colormap_notify_event_t XCBColormapNotifyEvent;
typedef xcb_colormap_notify_event_t XCBColorMapNotifyEvent;
typedef xcb_client_message_event_t XCBClientMessageEvent;
typedef xcb_property_notify_event_t XCBPropertyNotifyEvent;
typedef xcb_selection_clear_event_t XCBSelectionClearEvent;
typedef xcb_selection_notify_event_t XCBSelectionNotifyEvent;
typedef xcb_selection_request_event_t XCBSelectionRequestEvent;
/* This is NOT short for XCBGenericEvent rather is used for Ge Events */
typedef xcb_ge_event_t XCBGeEvent;


/* 
 * Opens the display returning a XCBDisplay * to the connection
 *
 *
 *
 * display_name:                Pass in as ":X" where X is the number of the display to connect to.              
 * *screen_number:              This returns the display screen number.
 * RETURN: NULL on Failure.
 * RETURN: XCBDisplay * on Success.
 *
 * NOTE: To check error use XCBGetErrorText() or see XCBCheckDisplayError() to get error number.
 */
XCBDisplay *
XCBOpenDisplay(
        const char *display_name, 
        int *screen_number_return);
/* 
 * Opens the display and returns a XCBDisplay* on Success.
 * Unchecked, caller must check for connection error using XCBCheckDisplayError();
 *
 *
 * display_name:                Pass in as ":X" where X is the number of the display to connect to.              
 * *screen_number:              This returns the display screen number.
 * RETURN: XCB Display * ;
 * RETURN: NULL on I/O error (rare)
 *
 * NOTE: This function will NEVER return NULL.
 * NOTE: To check error use XCBGetErrorText() or see XCBCheckDisplayError() to get error number.
 */
XCBDisplay *
XCBOpenConnection(
        const char *display_name,
        int *screen_number_return);
/* 
 * Closes the connection Specified and frees the data associated with connection.
 * No side effects if display is NULL.
 */
void 
XCBCloseDisplay(
        XCBDisplay *display);
/* 
 * display: Specifies the connection to the X server.
 * return a connection number for the specified display. 
 * On a POSIX-conformant system, this is the file descriptor of the connection.
 */
int 
XCBDisplayNumber(
        XCBDisplay *display);
/* 
 * display: Specifies the connection to the X server.
 * return a connection number for the specified display. 
 * On a POSIX-conformant system, this is the file descriptor of the connection.
 */
int 
XCBConnectionNumber(
        XCBDisplay *display);
/*
 * 
 */
XCBScreen *
XCBScreenOfDisplay(
        XCBDisplay *display, 
        int scren);
/*
 */
XCBScreen *
XCBDefaultScreenOfDisplay(
        XCBDisplay *display, 
        int screen);
/*
 * display: Specifies the connection to the X server.
 * return the number of available screens.
 */
int 
XCBScreenCount(
        XCBDisplay *display);
/*
 * return a pointer to a string that provides some identification of the owner of the X server implementation. If the data returned by the server is in the Latin Portable Character Encoding, then the string is in the Host Portable Character Encoding. Otherwise, the contents of the string are implementation dependent.
 * MUST BE FREED BY CALLING free()
 */
char *
XCBServerVendor(
        XCBDisplay *display);
/*  display Specifies the connection to the X server.
 *  return the major version number (11) of the X protocol associated with the connected display. 
 */
int 
XCBProtocolVersion(
        XCBDisplay *display);
/* display 	Specifies the connection to the X server.
 * return the minor protocol revision number of the X server.
 */
int 
XCBProtocolRevision(
        XCBDisplay *display);
/*
 * display 	Specifies the connection to the X server.
 * return a number related to a vendor's release of the X server.
 */
int 
XCBVendorRelease(
        XCBDisplay *display);

int 
XCBBitmapUnit(
        XCBDisplay *display);

int 
XCBBitmapBitOrder(
        XCBDisplay *display);

int 
XCBBitmapPad(
        XCBDisplay *display);

int 
XCBImageByteOrder(
        XCBDisplay *display);

int 
XCBRootOfScreen(
        XCBDisplay *display);

/* Gets the screen setup struct AKA screen stuff */
const XCBSetup *
XCBGetSetup(
        XCBDisplay *display);

XCBScreen *
XCBGetScreen(
        XCBDisplay *display);

/*
 * These are useful with functions that need a drawable of a particular screen and for creating top-level windows.
 * return the root window. 
 */
XCBWindow 
XCBRootWindow(
        XCBDisplay *display, 
        int screen);
/*
 */
XCBWindow 
XCBDefaultRootWindow(
        XCBDisplay *display, 
        int screen);
/*  return the width of the screen in pixels
 */
uint16_t 
XCBDisplayWidth(
        XCBDisplay *display, 
        int screen);
/* return an integer that describes the height of the screen in pixels. 
 */
uint16_t 
XCBDisplayHeight(
        XCBDisplay *display, 
        int screen);
uint8_t 
XCBDisplayDepth(
        XCBDisplay *display, 
        int screen);
uint8_t 
XCBDefaultDepth(
        XCBDisplay *display, 
        int screen);
XCBCookie
XCBSelectInput(
        XCBDisplay *display, 
        XCBWindow window, 
        uint32_t mask);
/*
 */
XCBCookie
XCBChangeWindowAttributes(
        XCBDisplay *display, 
        XCBWindow window, 
        uint32_t mask, 
        XCBWindowAttributes *window_attributes);

uint32_t 
XCBBlackPixel(
        XCBDisplay *display, 
        int screen);

uint32_t 
XCBWhitePixel(
        XCBDisplay *display, 
        int screen);

/* Syncs the current client to the XServer.
 *
 * This is generally a much faster approach than explicitly calling XCBFlush(); due to the lack of need to write to buffer(I/O)
 * However does not substitute in any way XCBFlush(); Use it instead when requiring a output;
 * 
 * Remember Syncing is cheap, Flushing is expensive.
 *
 * NOTE: Having too many request pending eg. not Syncing causes slow requests to the server, this is due to the buffer being full.
 *       Meaning you must sync the buffer with the XServer to send more, (eg. requests_pending > 1000).
 * NOTE: Flushing is mostly fine at requests_pending < 100, however syncing is still prefered when available.
 *       There would be a noticeable delay though when flushing at requests_pending > 200-1000.
 * NOTE: You can tests this on your system by sending alot of requests to the XServer and calling XFlush after x amount of requests.
 * NOTE: It is recommended to call XSync() after every ~1000 or so requests, if quickly sending alot of events.
 *       However if not sending alot of events in x < 1 (seconds) then syncing may not be nessesary.
 */
void 
XCBSync(
        XCBDisplay *display);

XCBCookie
XCBMoveWindow(
        XCBDisplay *display, 
        XCBWindow window, 
        int32_t x, 
        int32_t y);

XCBCookie
XCBMoveResizeWindow(
        XCBDisplay *display, 
        XCBWindow window, 
        int32_t x, 
        int32_t y, 
        uint32_t width, 
        uint32_t height);

XCBCookie
XCBRaiseWindow(
        XCBDisplay *display, 
        XCBWindow window);

XCBCookie
XCBMapRaised(
        XCBDisplay *display, 
        XCBWindow window);

XCBCookie
XCBLowerWindow(
        XCBDisplay *display, 
        XCBWindow window);

XCBCookie
XCBRaiseWindowIf(
        XCBDisplay *display, 
        XCBWindow window);

XCBCookie
XCBLowerWindowIf(
        XCBDisplay *display, 
        XCBWindow window);

XCBCookie
XCBSetWindowBorderWidth(
        XCBDisplay *display, 
        XCBWindow window, 
        uint32_t border_width);

XCBCookie
XCBSetSibling(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBWindow sibling);

XCBAtomCookie 
XCBInternAtomCookie(
        XCBDisplay *display, 
        const char *name, 
        int only_if_exists);

XCBAtom 
XCBInternAtomReply(
        XCBDisplay *display, 
        XCBAtomCookie cookie);

XCBWindowAttributesCookie 
XGetWindowAttributesCookie(
        XCBDisplay *display, 
        XCBWindow window);

XCBWindowAttributesReply *
XCBGetWindowAttributesReply(
        XCBDisplay *display, 
        XCBWindowAttributesCookie cookie);

XCBGeometryCookie 
XCBGetWindowGeometryCookie(
        XCBDisplay *display, 
        XCBWindow window);

XCBGeometry *
XCBGetWindowGeometryReply(
        XCBDisplay *display, 
        XCBGeometryCookie cookie);

XCBPixmap 
XCBCreatePixmap(
        XCBDisplay *display, 
        XCBWindow root, 
        uint16_t width, 
        uint16_t height, 
        uint8_t depth);

XCBCursor 
XCBCreateFontCursor(
        XCBDisplay *display, 
        int shape);

XCBCookie
XCBDefineCursor(
        XCBDisplay *display, 
        XCBWindow window, XCBCursor id);

XCBCookie
XCBFreeCursor(
        XCBDisplay *display, 
        XCBCursor cursor);

XCBCookie
XCBOpenFont(
        XCBDisplay *display, 
        XCBFont id, const char *name);

XCBCookie
XCBCloseFont(
        XCBDisplay *display,
        XCBFont id
        );

XCBPointerCookie 
XCBQueryPointerCookie(
        XCBDisplay *display, 
        XCBWindow window);

XCBPointerReply *
XCBQueryPointerReply(
        XCBDisplay *display, 
        XCBPointerCookie cookie);
/**/
/* text props */
XCBTextPropertyCookie 
XCBGetTextPropertyCookie(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBAtom property);
/* 
 * resulting data ON Success MUST be freed using XCBFreeTextProperty()
 * RETURN: 
 * 1 -> Success.
 * 0 -> Failure.
 */
int 
XCBGetTextPropertyReply(
        XCBDisplay *display, 
        XCBTextPropertyCookie cookie, 
        XCBTextProperty *reply_return);

/*
 * RETURN: 1 on Success.
 */
int
XCBFreeTextProperty(
        XCBTextProperty *prop);

/* 
 * Flushes buffered output to XServer.
 * Blocks Until buffer is fully flushed.
 *
 * !!!!!!IMPORTANT!!!!!!
 * This function should not be called often and should be called on important things that cant wait for XCBSync();
 *
 * NOTE: Flushing small buffers, e.g. requests_pending < 100 is slightly faster than calling XCBSync.
 *       however due to the nature of flushing this case rarely if ever occurs.
 * NOTE: Flushing large buffers e.g., requests_pending > 200-1000 is considerably slower than calling XCBSync.
 *       however yields faster results then calling neither.
 * 
 * CONCLUSION: Use XCBSync() when ever possible, and after every ~1000 requests_pending due to the Display buffer filling up.
 *             Though depending on use case XCBSync may never need to be called, if there arent enough requests_pending to fill up Display buffer.
 *
 * RETURN: 0 on Success.
 * RETURN: 1 on Failure.
 */ 
int 
XCBFlush(
        XCBDisplay *display);

/*
 * Gets the maximum data that a XCBDisplay* can hold in bytes / 4.
 * Ex: 4 bytes size would return 1
 * 
 * RANGE:
 *                    (256kiB)
 * NO_EXTENSION -> (0, 65535)
 * 
 * //COMMENT, Im not sure on this one docu is vague 
 *                    (16GB)
 * BIG_REQUESTS -> (0, 2097152)
 * 
 * RETURN: Max Request length in (bytes / 4)
 */
uint32_t
XCBGetMaximumRequestLength(
        XCBDisplay *display);



/* Check if the display flag has set a connection error display->has_error;
 * 
 * RETURN: XCB_CONN_ERROR, because of socket errors, pipe errors or other stream errors.
 * RETURN: XCB_CONN_CLOSED_EXT_NOTSUPPORTED, when extension not supported.
 * RETURN: XCB_CONN_CLOSED_MEM_INSUFFICIENT, when memory not available.
 * RETURN: XCB_CONN_CLOSED_REQ_LEN_EXCEED, exceeding request length that server accepts.
 * RETURN: XCB_CONN_CLOSED_PARSE_ERR, error during parsing display string.
 * RETURN: XCB_CONN_CLOSED_INVALID_SCREEN, because the server does not have a screen matching the display.
 */
int 
XCBCheckDisplayError(
        XCBDisplay *display);
/* Check if the display flag has set a connection error display->has_error;
 * 
 * RETURN: XCB_CONN_ERROR, because of socket errors, pipe errors or other stream errors.
 * RETURN: XCB_CONN_CLOSED_EXT_NOTSUPPORTED, when extension not supported.
 * RETURN: XCB_CONN_CLOSED_MEM_INSUFFICIENT, when memory not available.
 * RETURN: XCB_CONN_CLOSED_REQ_LEN_EXCEED, exceeding request length that server accepts.
 * RETURN: XCB_CONN_CLOSED_PARSE_ERR, error during parsing display string.
 * RETURN: XCB_CONN_CLOSED_INVALID_SCREEN, because the server does not have a screen matching the display.
 */
int
XCBCheckDisplayError(
        XCBDisplay *display);
/* Check if the display flag has set a connection error display->has_error;
 * 
 * RETURN: XCB_CONN_ERROR, because of socket errors, pipe errors or other stream errors.
 * RETURN: XCB_CONN_CLOSED_EXT_NOTSUPPORTED, when extension not supported.
 * RETURN: XCB_CONN_CLOSED_MEM_INSUFFICIENT, when memory not available.
 * RETURN: XCB_CONN_CLOSED_REQ_LEN_EXCEED, exceeding request length that server accepts.
 * RETURN: XCB_CONN_CLOSED_PARSE_ERR, error during parsing display string.
 * RETURN: XCB_CONN_CLOSED_INVALID_SCREEN, because the server does not have a screen matching the display.
 */
int 
XCBHasConnectionError(
        XCBDisplay *display);
/* Check if the display flag has set a connection error display->has_error;
 * 
 * RETURN: XCB_CONN_ERROR, because of socket errors, pipe errors or other stream errors.
 * RETURN: XCB_CONN_CLOSED_EXT_NOTSUPPORTED, when extension not supported.
 * RETURN: XCB_CONN_CLOSED_MEM_INSUFFICIENT, when memory not available.
 * RETURN: XCB_CONN_CLOSED_REQ_LEN_EXCEED, exceeding request length that server accepts.
 * RETURN: XCB_CONN_CLOSED_PARSE_ERR, error during parsing display string.
 * RETURN: XCB_CONN_CLOSED_INVALID_SCREEN, because the server does not have a screen matching the display.
 */
int 
XCBHasDisplayError(
        XCBDisplay *display);


/* NONFUNCTIONING
 * 1 -> Error handler set.
 * 0 -> Error handler unset.
 * Incase of an unset error handler (default) XCB simply calls die() when an error occurs which may not be desired.
 * One should note that this function simply sets the function to be called when an error occurs using this API.
 * Meaning that this only handles calls made by this API, this does not handle any errors caused by another thread or raw xcb calls.
 * RETURN: {1, 0}.
 */
int 
XCBSetErrorHandler(void (*error_handler)(XCBDisplay *, XCBGenericError *));


/* not implemented */
void 
XCBSetIOErrorHandler(
        XCBDisplay *display, 
        void *IOHandler);

/* TODO */
char *
XCBGetErrorText(
        XCBDisplay *display);







/* events */


/* 
 * Gets and returns the next Event from the XServer.
 * This returns a structure called xcb_generic_event_t.
 * This Function Blocks until a request is received.
 *
 * event_return: XCBGenericEvent * on Success.
 * event_return: XCBGenericError * on Error.
 * event_return: NULL on I/O Error.
 *
 * RETURN: 1 On Success.
 * RETURN: 0 On Failure.
 */
int
XCBNextEvent(
        XCBDisplay *display, 
        XCBGenericEvent **event_return);
/* 
 * Gets and returns the next Event from the XServer.
 * This returns a structure called xcb_generic_event_t.
 * This Function Blocks until a request is received.
 *
 * RETURN: XCBGenericEvent * on Success.
 * RETURN: XCBGenericError * on Error.
 * RETURN: NULL on I/O Error.
 */
XCBGenericEvent *
XCBWaitForEvent(
        XCBDisplay *display);

/**
 * @brief Returns the next event or error from the server.
 * @param c The connection to the X server.
 * @return The next event from the server.
 *
 * Returns the next event or error from the server, if one is
 * available, or returns @c NULL otherwise. If no event is available, that
 * might be because an I/O error like connection close occurred while
 * attempting to read the next event, in which case the connection is
 * shut down when this function returns.
 *
 * NOTE: XCBGenericEvent event_type
 */

XCBGenericEvent *
XCBPollForEvent(
        XCBDisplay *display);
/** <Straight from the documentation.>
 * @brief Returns the next event without reading from the connection.
 * @param c The connection to the X server.
 * @return The next already queued event from the server.
 *
 * This is a version of xcb_poll_for_event that only examines the
 * event queue for new events. The function doesn't try to read new
 * events from the connection if no queued events are found.
 *
 * This function is useful for callers that know in advance that all
 * interesting events have already been read from the connection. For
 * example, callers might use xcb_wait_for_reply and be interested
 * only of events that preceded a specific reply.
 */
XCBGenericEvent *
XCBPollForQueuedEvent(
        XCBDisplay *display);
/* Check if a specified cookie request has a reply available from the XServer.
 * 
 * RETURN: 1 On Success.
 * RETURN: 0 On Not Avaible.
 */
int 
XCBPollForReply(
        XCBDisplay *display, 
        XCBCookie request, 
        void **reply_return);
/* Check if a specified cookie request has a reply available from the XServer.
 * This is different from XCBPollForReply() as it assumes the request has already be widened.
 *
 * RETURN: 1 On Success.
 * RETURN: 0 On Not Avaible/Failure.
 */
int 
XCBPollForReply64(
        XCBDisplay *display, 
        XCBCookie request, 
        void **reply);
/* Check if a specified cookie request has a reply available from the XServer.
 * 
 * RETURN: NULL On Not Avaible/Failure.
 * RETURN: void * On Success.
 */
void *
XCBCheckReply(
        XCBDisplay *display, 
        XCBCookie request);
/* Check if a specified cookie request has a reply available from the XServer.
 * This is different from XCBCheckReply() as it assumes the request has already be widened.
 *
 * RETURN: NULL On not Avaible/Failure.
 * RETURN: void * On Success.
 */
void *
XCBCheckReply64(
        XCBDisplay *display, 
        XCBCookie request);

/* grabbing/grab */
/*
 * key:             XCB_GRAB_ANY                Grab all possible key codes.
 *                  XCBKeyCode                  The keycode of the specified key combination.
 *
 * modifiers:       XCB_MOD_MASK_ANY            Release all key combinations regardless of modifier.
 *                  XCB_MOD_MASK_SHIFT          The Shift Key. 
 *                  XCB_MOD_MASK_LOCK           The Lock Key.
 *                  XCB_MOD_MASK_CONTROL        The Ctrl Key.
 *                  XCB_MOD_MASK_1              The Alt Key.
 *                  XCB_MOD_MASK_2              The Numlock Key.
 *                  XCB_MOD_MASK_3              ISO_LEVEL5_SHIFT              
 *                  XCB_MOD_MASK_4              The Super/Windows Key.
 *                  XCB_MOD_MASK_5              ISO_LEVEL3_SHIFT 
 *
 * grab_window:     XCBWindow                   The window on which the grabbed key combination will be grabbed.
 *
 * owner_events:    0/false/False               The Key Events are not reported to the grab_window.
 *                  1/true/True                 The Key Events are reported to the grab_window.
 * 
 * pointer_mode:    XCB_GRAB_MODE_SYNC          The state of the keyboard appears to freeze: No further keyboard events are.
 *                                              generated by the server until the grabbing client issues a releasing.
 *                                              `AllowEvents` request or until the keyboard grab is released.
 *                  XCB_GRAB_MODE_ASYNC         Keyboard event processing continues normally.
 *
 * keyboard_mode:   XCB_GRAB_MODE_SYNC          The state of the keyboard appears to freeze: No further keyboard events are.
 *                                              generated by the server until the grabbing client issues a releasing.
 *                                              `AllowEvents` request or until the keyboard grab is released.
 *                  XCB_GRAB_MODE_ASYNC         Keyboard event processing continues normally.
 *
 * RETURN: Cookie to request.
 */
XCBCookie
XCBGrabKey(
        XCBDisplay *display, 
        XCBKeyCode key, 
        uint16_t modifiers, 
        XCBWindow grab_window, 
        uint8_t owner_events, 
        uint8_t pointer_mode, 
        uint8_t keyboard_mode);
/*  
 * keycode:         XCB_GRAB_ANY                Release all possible key codes.
 *                  XCBKeyCode                  The keycode of the specified key combination.
 * modifiers:       XCB_MOD_MASK_ANY            Release all key combinations regardless of modifier.
 *                  XCB_MOD_MASK_SHIFT          The Shift Key. 
 *                  XCB_MOD_MASK_LOCK           The Lock Key.
 *                  XCB_MOD_MASK_CONTROL        The Ctrl Key.
 *                  XCB_MOD_MASK_1              The Alt Key.
 *                  XCB_MOD_MASK_2              The Numlock Key.
 *                  XCB_MOD_MASK_3              ISO_LEVEL5_SHIFT              
 *                  XCB_MOD_MASK_4              The Super/Windows Key.
 *                  XCB_MOD_MASK_5              ISO_LEVEL3_SHIFT 
 *
 * grab_window:     XCBWindow                   The window on which the grabbed key combination will be released.
 *
 * RETURN: Cookie to request.
 */
XCBCookie
XCBUngrabKey(
        XCBDisplay *display,
        XCBKeyCode key,
        uint16_t modifiers,
        XCBWindow grab_window
        );
/* button:      XCB_BUTTON_INDEX_ANY        Any of the following (or none).
 *              XCB_BUTTON_INDEX_1          The left mouse button.
 *              XCB_BUTTON_INDEX_2          The right mouse button.
 *              XCB_BUTTON_INDEX_3          The middle mouse button.
 *              XCB_BUTTON_INDEX_4          The Scroll Wheel. (direction TODO).
 *              XCB_BUTTON_INDEX_5          The Scroll Wheel. (direction TODO).
 *
 * modifier:    XCB_BUTTON_MASK_1           Not enough information
 *              XCB_BUTTON_MASK_2           Not enough information
 *              XCB_BUTTON_MASK_3           Not enough information
 *              XCB_BUTTON_MASK_4           Not enough information.
 *              XCB_BUTTON_MASK_5           Not enough information.
 *              XCB_BUTTON_MASK_ANY         This is guaranteed to be a mask.
 *  
 *  NOTE: ''uint8_t button' IS NOT a mask.
 *  NOTE: 'modifier' is undocumented.
 *
 *  RETURN: Cookie to request.
 */
XCBCookie
XCBUngrabButton(
        XCBDisplay *display, 
        uint8_t button, 
        uint16_t modifier, 
        XCBWindow window
        );
/*  
 * button:          XCB_BUTTON_INDEX_ANY        Any of the following (or none).
 *                  XCB_BUTTON_INDEX_1          The left mouse button.
 *                  XCB_BUTTON_INDEX_2          The right mouse button.
 *                  XCB_BUTTON_INDEX_3          The middle mouse button.
 *                  XCB_BUTTON_INDEX_4          The Scroll Wheel. (direction TODO).
 *                  XCB_BUTTON_INDEX_5          The Scroll Wheel. (direction TODO).
 *
 * modifiers:       XCB_MOD_MASK_SHIFT          The Shift Key. 
 *                  XCB_MOD_MASK_LOCK           The Lock Key.
 *                  XCB_MOD_MASK_CONTROL        The Ctrl Key.
 *                  XCB_MOD_MASK_1              The Alt Key.
 *                  XCB_MOD_MASK_2              The Numlock Key.
 *                  XCB_MOD_MASK_3              ISO_LEVEL5_SHIFT              
 *                  XCB_MOD_MASK_4              The Super/Windows Key.
 *                  XCB_MOD_MASK_5              ISO_LEVEL3_SHIFT 
 *                  XCB_MOD_MASK_ANY
 *
 * grab_window:     XCBWindow                   Specifies the window on which the pointer should be grabbed.
 *
 * owner_events:    1/true/True                 Report grabbed events to grab_window.
 *                  0/false/False               Dont report grabbed events to grab_window.
 *
 * event_mask:      XCB_EVENT_MASK_(...)        Specifies which pointer events are reported to the client.
 *
 * pointer_mode:    XCB_GRAB_MODE_SYNC          The state of the keyboard appears to freeze: No further keyboard events are.
 *                                              generated by the server until the grabbing client issues a releasing.
 *                                              `AllowEvents` request or until the keyboard grab is released.
 *                  XCB_GRAB_MODE_ASYNC         Keyboard event processing continues normally.
 *
 * keyboard_mode:   XCB_GRAB_MODE_SYNC          The state of the keyboard appears to freeze: No further keyboard events are.
 *                                              generated by the server until the grabbing client issues a releasing.
 *                                              `AllowEvents` request or until the keyboard grab is released.
 *                  XCB_GRAB_MODE_ASYNC         Keyboard event processing continues normally.
 *
 * window_confide:  XCB_WINDOW_NONE/XCB_NONE    Dont confide to window.
 *                  XCBWindow                   Specifies the window to confine the pointer in (the user will not be able to.
 *                                              move the pointer out of that window).
 * cursor:          XCB_CURSOR_NONE/XCB_NONE    Dont change the cursor/leave it as it was.
 *                  XCBCursor                   Specifies the cursor that should be displayed.
 * RETURN: Cookie to request.
 */
XCBCookie
XCBGrabButton(
        XCBDisplay *display, 
        uint8_t button, 
        uint16_t modifiers, 
        XCBWindow grab_window, 
        uint8_t owner_events, 
        uint16_t event_mask,
        uint8_t pointer_mode,
        uint8_t keyboard_mode,
        XCBWindow window_confide,
        XCBCursor cursor 
        );
/* Returns min-keycodes and max-keycodes supported by the specified display.
 * The minimum number of KeyCodes returned is never less than 8, and the maximum number of KeyCodes returned is never greater than 255. 
 * Not all KeyCodes in this range are required to have corresponding keys.
 *
 * RETURN: 1 always;
 */
int 
XCBDisplayKeyCodes(
        XCBDisplay *display, 
        int *min_keycode_return, 
        int *max_keycode_return);
/* Returns min-keycodes and max-keycodes supported by the specified display.
 * The minimum number of KeyCodes returned is never less than 8, and the maximum number of KeyCodes returned is never greater than 255. 
 * Not all KeyCodes in this range are required to have corresponding keys.
 *
 * RETURN: 1 always;
 */
int 
XCBDisplayKeycodes(
        XCBDisplay *display, 
        int *min_keycode_return, 
        int *max_keycode_return);

/*
 * NOTE: RETURN MUST BE RELEASED BY CALLER USING free().
 */
XCBKeycode *
XCBGetKeycodes(XCBDisplay *display, XCBKeysym keysym);
/*
 * NOTE: RETURN MUST BE RELEASED BY CALLER USING free().
 */
XCBKeyCode *
XCBGetKeyCodes(XCBDisplay *display, XCBKeysym keysym);


XCBKeyboardMappingCookie 
XCBGetKeyboardMappingCookie(
        XCBDisplay *display, 
        XCBKeyCode first_keycode, 
        uint8_t count);

XCBKeyboardMapping *
XCBGetKeyboardMappingReply(
        XCBDisplay *display, 
        XCBKeyboardMappingCookie cookie);

/* Send a event to the XServer to map the window specified;
 *
 * RETURN: Cookie to request.
 */
XCBCookie
XCBMapWindow(
        XCBDisplay *display, 
        XCBWindow window);

/* windows*/
XCBWindow 
XCBCreateWindow(
        XCBDisplay *display, 
        XCBWindow parent, 
        int16_t x, 
        int16_t y, 
        uint16_t width, 
        uint16_t height, 
        uint16_t border_width, 
        uint8_t depth, 
        unsigned int _class, 
        XCBVisual visual, 
        uint32_t valuemask, 
        const uint32_t *value_list);
XCBWindow
XCBCreateSimpleWindow(
        XCBDisplay *display,
        XCBWindow parent,
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint16_t border_width,
        uint32_t border,
        uint32_t background
        );


/* GC */
/* RETURN: GC id (identification number) */
XCBGC 
XCBCreateGC(
        XCBDisplay *display, 
        XCBDrawable drawable, 
        uint32_t valuemask, 
        const void *valuelist);
/* 
 * linewidth: measured in pixels and can be greater than or equal to one, a wide line, or the special value zero, a thin line.
 * linestyle: XCB_LINE_STYLE_SOLID          The full path of the line is drawn.
 *
 *            XCB_LINE_STYLE_DOUBLE_DASH    The full path of the line is drawn, 
 *                                          but the even dashes are filled differently than the odd dashes (see fill-style), 
 *                                          with Butt cap-style used where even and odd dashes meet.
 *
 *            XCB_LINE_STYLE_ON_OFF_DASH    Only the even dashes are drawn, 
 *                                          and cap-style applies to all internal ends of the individual dashes (except NotLast is treated as Butt).
 *
 * capstyle: XCB_CAP_STYLE_NOT_LAST         The result is equivalent to Butt, except that for a line-width of zero the final endpoint is not drawn.
 *           XCB_CAP_STYLE_BUTT             The result is square at the endpoint (perpendicular to the slope of the line) with no projection beyond.
 *           XCB_CAP_STYLE_ROUND            The result is a circular arc with its diameter equal to the line-width, centered
 *                                          on the endpoint; it is equivalent to Butt for line-width zero.
 *           XCB_CAP_STYLE_PROJECTING       The result is square at the end, but the path continues beyond the endpoint for
 *                                          a distance equal to half the line-width; it is equivalent to Butt for line-width zero.
 *
 * joinstyle: XCB_JOIN_STYLE_MITER          The outer edges of the two lines extend to meet at an angle. However, if the
 *                                          angle is less than 11 degrees, a Bevel join-style is used instead.
 *            XCB_JOIN_STYLE_ROUND          The result is a circular arc with a diameter equal to the line-width, centered
 *                                          on the joinpoint.
 *            XCB_JOIN_STYLE_BEVEL          The result is Butt endpoint styles, and then the triangular notch is filled.
 *
 *
 * RETURN: 1, always cause why not 
 */
int 
XCBSetLineAttributes(
        XCBDisplay *display, 
        XCBGC gc, 
        uint32_t linewidth, 
        uint32_t linestyle, 
        uint32_t capstyle, 
        uint32_t joinstyle);

/*
 * property:                                    The atom property AKA XInternAtom(dpy, "my_cool_atom", False);
 *
 * type:        XCBAtom                         Not often the case but specific atom require this, see above for type.
 *              XCBAtomType                     The XCB_ATOM_(sometype) of the property. (Xlib XA_(sometype))
 *
 * format:                                      Specifies whether the data should be viewed as a list of 8-bit, 16-bit or.
 *                                              32-bit quantities. Possible values are 8, 16 and 32. This information allows.
 *                                              the X server to correctly perform byte-swap operations as necessary.
 *
 * mode:        XCB_PROP_MODE_REPLACE           Discard the previous property value and store the new data.
 *
 *              XCB_PROP_MODE_PREPEND           Insert the new data before the beginning of existing data. The `format` must.
 *                                              match existing property value. If the property is undefined, it is treated as.
 *                                              defined with the correct type and format with zero-length data.
 *
 *              XCB_PROP_MODE_APPEND            Insert the new data after the beginning of existing data. The `format` must.
 *                                              match existing property value. If the property is undefined, it is treated as.
 *                                              defined with the correct type and format with zero-length data.
 * data:                                        The property data.
 * 
 * nelements:                                   Specifies the number of elements.
 * RETURN: Cookie to request.
*/
XCBCookie
XCBChangeProperty(
        XCBDisplay *display,
        XCBWindow window, 
        XCBAtom property, 
        XCBAtom type, 
        uint8_t format, 
        uint8_t mode, 
        const void *data, 
        uint32_t nelements); /* data_len */
/*
 * RETURN: Cookie to request.
*/
XCBCookie
XCBDeleteProperty(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBAtom property);

/* Mask;
 * value_mask:          XCB_CONFIG_WINDOW_X;            Sets the flag to configure the X axis.
 *                      XCB_CONFIG_WINDOW_Y;            Sets the flag to configure the Y axis.
 *                      XCB_CONFIG_WINDOW_WIDTH;        Sets the flag to configure the Width.
 *                      XCB_CONFIG_WINDOW_HEIGHT;       Sets the flag to configure the Height.
 *                      XCB_CONFIG_WINDOW_BORDER_WIDTH; Sets the flag to configure the Border Width.
 *                      XCB_CONFIG_WINDOW_SIBLING;      Sets the flag to configure the Window Sibling.
 *                      XCB_CONFIG_WINDOW_STACK_MODE;   Sets the flag to configure the Stack Mode.
 * 
 * changes:             XCBWindowChanges *              The structure to the changes.
 * RETURN: Cookie to request.
*/
XCBCookie
XCBConfigureWindow(
        XCBDisplay *display, 
        XCBWindow window,
        uint16_t value_mask,
        XCBWindowChanges *changes);
/* Sets the classhint for a specified window with class_name as the name hint similiar to XSetClassHint()
 *
 * NOTE: class_name does not protect against non terminating strings.
 *
 * RETURN: A Cookie to request.
 */
XCBCookie
XCBSetClassHint(
        XCBDisplay *display, 
        XCBWindow window, 
        const char *class_name);

/* Valuemasks
XCB_GC_FUNCTION
XCB_GC_PLANE_MASK
XCB_GC_FOREGROUND
XCB_GC_BACKGROUND
XCB_GC_LINE_WIDTH
XCB_GC_LINE_STYLE
XCB_GC_CAP_STYLE
XCB_GC_JOIN_STYLE
XCB_GC_FILL_STYLE
XCB_GC_FILL_RULE
XCB_GC_TILE
XCB_GC_STIPPLE
XCB_GC_TILE_STIPPLE_ORIGIN_X
XCB_GC_TILE_STIPPLE_ORIGIN_Y
XCB_GC_FONT
XCB_GC_SUBWINDOW_MODE
XCB_GC_GRAPHICS_EXPOSURES
XCB_GC_CLIP_ORIGIN_X
XCB_GC_CLIP_ORIGIN_Y
XCB_GC_CLIP_MASK
XCB_GC_DASH_OFFSET
XCB_GC_DASH_LIST
XCB_GC_ARC_MODE
*/
XCBCookie
XCBChangeGC(
        XCBDisplay *display, 
        XCBGC gc, 
        uint32_t valuemask, 
        const void *valuelist);

XCBCookie
XCBDrawPoint(
        XCBDisplay *display,
        uint8_t coordinate_mode,
        XCBDrawable drawable,
        XCBGC gc,
        uint32_t points_len,
        XCBPoint *points
        );
int
XCBDiscardReply(
        XCBDisplay *display, 
        XCBCookie cookie);

/* dumb stuff */


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
void 
XCBPrefetchMaximumRequestLength(
        XCBDisplay *display);



/* Events */




/* 1356 xproto.h */
/* MASKS */
/*
 * modifier 
 * -----------------
 * XCB_MOD_MASK_SHIFT = 1,
 * XCB_MOD_MASK_LOCK = 2,
 * XCB_MOD_MASK_CONTROL = 4,
 * XCB_MOD_MASK_1 = 8,
 * XCB_MOD_MASK_2 = 16,
 * XCB_MOD_MASK_3 = 32,
 * XCB_MOD_MASK_4 = 64,
 * XCB_MOD_MASK_5 = 128,
 * XCB_MOD_MASK_ANY = 32768
 *
 * Key
 * ---------------------
 * XCB_KEY_BUT_MASK_SHIFT = 1,
 * XCB_KEY_BUT_MASK_LOCK = 2,
 * XCB_KEY_BUT_MASK_CONTROL = 4,
 * XCB_KEY_BUT_MASK_MOD_1 = 8,
 * XCB_KEY_BUT_MASK_MOD_2 = 16,
 * XCB_KEY_BUT_MASK_MOD_3 = 32,
 * XCB_KEY_BUT_MASK_MOD_4 = 64,
 * XCB_KEY_BUT_MASK_MOD_5 = 128,
 * XCB_KEY_BUT_MASK_BUTTON_1 = 256,
 * XCB_KEY_BUT_MASK_BUTTON_2 = 512,
 * XCB_KEY_BUT_MASK_BUTTON_3 = 1024,
 * XCB_KEY_BUT_MASK_BUTTON_4 = 2048,
 * XCB_KEY_BUT_MASK_BUTTON_5 = 4096
 *
 * Button
 * ----------------------
 *
 * XCB_BUTTON_MASK_1 = 256,
 * XCB_BUTTON_MASK_2 = 512,
 * XCB_BUTTON_MASK_3 = 1024,
 * XCB_BUTTON_MASK_4 = 2048,
 * XCB_BUTTON_MASK_5 = 4096,
 * XCB_BUTTON_MASK_ANY = 32768
 *
 * Visual Class 
 * ---------------------
 * XCB_VISUAL_CLASS_STATIC_GRAY = 0,
 * XCB_VISUAL_CLASS_GRAY_SCALE = 1,
 * XCB_VISUAL_CLASS_STATIC_COLOR = 2,
 * XCB_VISUAL_CLASS_PSEUDO_COLOR = 3,
 * XCB_VISUAL_CLASS_TRUE_COLOR = 4,
 * XCB_VISUAL_CLASS_DIRECT_COLOR = 5
 *
 * Event 
 * --------------------
 * XCB_EVENT_MASK_NO_EVENT = 0,
 * XCB_EVENT_MASK_KEY_PRESS = 1,
 * XCB_EVENT_MASK_KEY_RELEASE = 2,
 * XCB_EVENT_MASK_BUTTON_PRESS = 4,
 * XCB_EVENT_MASK_BUTTON_RELEASE = 8,
 * XCB_EVENT_MASK_ENTER_WINDOW = 16,
 * XCB_EVENT_MASK_LEAVE_WINDOW = 32,
 * XCB_EVENT_MASK_POINTER_MOTION = 64,
 * XCB_EVENT_MASK_POINTER_MOTION_HINT = 128,
 * XCB_EVENT_MASK_BUTTON_1_MOTION = 256,
 * XCB_EVENT_MASK_BUTTON_2_MOTION = 512,
 * XCB_EVENT_MASK_BUTTON_3_MOTION = 1024,
 * XCB_EVENT_MASK_BUTTON_4_MOTION = 2048,
 * XCB_EVENT_MASK_BUTTON_5_MOTION = 4096,
 * XCB_EVENT_MASK_BUTTON_MOTION = 8192,
 * XCB_EVENT_MASK_KEYMAP_STATE = 16384,
 * XCB_EVENT_MASK_EXPOSURE = 32768,
 * XCB_EVENT_MASK_VISIBILITY_CHANGE = 65536,
 * XCB_EVENT_MASK_STRUCTURE_NOTIFY = 131072,
 * XCB_EVENT_MASK_RESIZE_REDIRECT = 262144,
 * XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY = 524288,
 * XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT = 1048576,
 * XCB_EVENT_MASK_FOCUS_CHANGE = 2097152,
 * XCB_EVENT_MASK_PROPERTY_CHANGE = 4194304,
 * XCB_EVENT_MASK_COLOR_MAP_CHANGE = 8388608,
 * XCB_EVENT_MASK_OWNER_GRAB_BUTTON = 16777216
 *
 * Motion
 * -------------------------------
 * XCB_MOTION_NORMAL = 0,
 * XCB_MOTION_HINT = 1
 *
 * Notify Detail
 * ------------------------------
 * XCB_NOTIFY_DETAIL_ANCESTOR = 0,
 * XCB_NOTIFY_DETAIL_VIRTUAL = 1,
 * XCB_NOTIFY_DETAIL_INFERIOR = 2,
 * XCB_NOTIFY_DETAIL_NONLINEAR = 3,
 * XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL = 4,
 * XCB_NOTIFY_DETAIL_POINTER = 5,
 * XCB_NOTIFY_DETAIL_POINTER_ROOT = 6,
 * XCB_NOTIFY_DETAIL_NONE = 7
 *
 * Notify Mode
 * -----------------------------
 * XCB_NOTIFY_MODE_NORMAL = 0,
 * XCB_NOTIFY_MODE_GRAB = 1,
 * XCB_NOTIFY_MODE_UNGRAB = 2,
 * XCB_NOTIFY_MODE_WHILE_GRABBED = 3
 *
 * Visiblity 
 * ----------------------------
 * XCB_VISIBILITY_UNOBSCURED = 0,
 * XCB_VISIBILITY_PARTIALLY_OBSCURED = 1,
 * XCB_VISIBILITY_FULLY_OBSCURED = 2
 *
 * Place
 * ----------------------------
 * XCB_PLACE_ON_TOP = 0,
 * XCB_PLACE_ON_BOTTOM = 1
 *
 * Property
 * ----------------------------
 * XCB_PROPERTY_NEW_VALUE = 0,
 * XCB_PROPERTY_DELETE = 1
 *
 * Time
 * ----------------------------
 * XCB_TIME_CURRENT_TIME = 0
 *
 * Atom (Comparable to Xlib's XA_ properties, which is short of X_ATOM which is short for XSERVER_ATOM)
 * ----------------------------
 * XCB_ATOM_NONE = 0,
 * XCB_ATOM_ANY = 0,
 * XCB_ATOM_PRIMARY = 1,
 * XCB_ATOM_SECONDARY = 2,
 * XCB_ATOM_ARC = 3,
 * XCB_ATOM_ATOM = 4,
 * XCB_ATOM_BITMAP = 5,
 * XCB_ATOM_CARDINAL = 6,
 * XCB_ATOM_COLORMAP = 7,
 * XCB_ATOM_CURSOR = 8,
 * XCB_ATOM_CUT_BUFFER0 = 9,
 * XCB_ATOM_CUT_BUFFER1 = 10,
 * XCB_ATOM_CUT_BUFFER2 = 11,
 * XCB_ATOM_CUT_BUFFER3 = 12,
 * XCB_ATOM_CUT_BUFFER4 = 13,
 * XCB_ATOM_CUT_BUFFER5 = 14,
 * XCB_ATOM_CUT_BUFFER6 = 15,
 * XCB_ATOM_CUT_BUFFER7 = 16,
 * XCB_ATOM_DRAWABLE = 17,
 * XCB_ATOM_FONT = 18,
 * XCB_ATOM_INTEGER = 19,
 * XCB_ATOM_PIXMAP = 20,
 * XCB_ATOM_POINT = 21,
 * XCB_ATOM_RECTANGLE = 22,
 * XCB_ATOM_RESOURCE_MANAGER = 23,
 * XCB_ATOM_RGB_COLOR_MAP = 24,
 * XCB_ATOM_RGB_BEST_MAP = 25,
 * XCB_ATOM_RGB_BLUE_MAP = 26,
 * XCB_ATOM_RGB_DEFAULT_MAP = 27,
 * XCB_ATOM_RGB_GRAY_MAP = 28,
 * XCB_ATOM_RGB_GREEN_MAP = 29,
 * XCB_ATOM_RGB_RED_MAP = 30,
 * XCB_ATOM_STRING = 31,
 * XCB_ATOM_VISUALID = 32,
 * XCB_ATOM_WINDOW = 33,
 * XCB_ATOM_WM_COMMAND = 34,
 * XCB_ATOM_WM_HINTS = 35,
 * XCB_ATOM_WM_CLIENT_MACHINE = 36,
 * XCB_ATOM_WM_ICON_NAME = 37,
 * XCB_ATOM_WM_ICON_SIZE = 38,
 * XCB_ATOM_WM_NAME = 39,
 * XCB_ATOM_WM_NORMAL_HINTS = 40,
 * XCB_ATOM_WM_SIZE_HINTS = 41,
 * XCB_ATOM_WM_ZOOM_HINTS = 42,
 * XCB_ATOM_MIN_SPACE = 43,
 * XCB_ATOM_NORM_SPACE = 44,
 * XCB_ATOM_MAX_SPACE = 45,
 * XCB_ATOM_END_SPACE = 46,
 * XCB_ATOM_SUPERSCRIPT_X = 47,
 * XCB_ATOM_SUPERSCRIPT_Y = 48,
 * XCB_ATOM_SUBSCRIPT_X = 49,
 * XCB_ATOM_SUBSCRIPT_Y = 50,
 * XCB_ATOM_UNDERLINE_POSITION = 51,
 * XCB_ATOM_UNDERLINE_THICKNESS = 52,
 * XCB_ATOM_STRIKEOUT_ASCENT = 53,
 * XCB_ATOM_STRIKEOUT_DESCENT = 54,
 * XCB_ATOM_ITALIC_ANGLE = 55,
 * XCB_ATOM_X_HEIGHT = 56,
 * XCB_ATOM_QUAD_WIDTH = 57,
 * XCB_ATOM_WEIGHT = 58,
 * XCB_ATOM_POINT_SIZE = 59,
 * XCB_ATOM_RESOLUTION = 60,
 * XCB_ATOM_COPYRIGHT = 61,
 * XCB_ATOM_NOTICE = 62,
 * XCB_ATOM_FONT_NAME = 63,
 * XCB_ATOM_FAMILY_NAME = 64,
 * XCB_ATOM_FULL_NAME = 65,
 * XCB_ATOM_CAP_HEIGHT = 66,
 *
 * Colormap State
 * ------------------------------
 * XCB_COLORMAP_STATE_UNINSTALLED = 0,
 * XCB_COLORMAP_STATE_INSTALLED = 1
 * 
 * Colormap
 * ------------------------------
 * XCB_COLORMAP_NONE = 0
 *
 * Mapping
 * ------------------------------
 * XCB_MAPPING_MODIFIER = 0,
 * XCB_MAPPING_KEYBOARD = 1,
 * XCB_MAPPING_POINTER = 2
 *
 * Window Class
 * ------------------------------
 * XCB_WINDOW_CLASS_COPY_FROM_PARENT = 0,
 * XCB_WINDOW_CLASS_INPUT_OUTPUT = 1,
 * XCB_WINDOW_CLASS_INPUT_ONLY = 2
 *
 *
 *
 * Backing store
 * ------------------------------
 * XCB_BACKING_STORE_NOT_USEFUL = 0,
 * XCB_BACKING_STORE_WHEN_MAPPED = 1,
 * XCB_BACKING_STORE_ALWAYS = 2
 *
 * Image order
 * --------------------------
 * XCB_IMAGE_ORDER_LSB_FIRST = 0,
 * XCB_IMAGE_ORDER_MSB_FIRST = 1
 *
 * Window
 * -----------------------
 * XCB_WINDOW_NONE = 0,
 *
*/

#endif
