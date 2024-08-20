#ifndef XCB_WINUTIL_H
#define XCB_WINUTIL_H



#include "xcb_trl.h"

enum NETWMPROTOCOLS
{
    /* Root window properties */
    NetSupported, 
    NetClientList, 
    NetClientListStacking,
    NetNumberOfDesktops, NetDesktopGeometry,
    NetDesktopViewport, NetCurrentDesktop,
    NetDesktopNames, NetActiveWindow,
    NetWorkarea, NetSupportingWMCheck,
    NetVirtualRoots, NetDesktopLayout,
    NetShowingDesktop,
    /* other root messages */
    NetCloseWindow, NetMoveResizeWindow, 
    NetMoveResize,/* _NET_WM_MOVERESIZE */
    NetRestackWindow,
    NetRequestFrameExtents,
    /* application win properties */
    NetWMName, NetWMVisibleName,
    NetWMIconName, NetWMVisibleIconName,
    NetWMDesktop, NetWMWindowType, 
    NetWMState, NetWMAllowedActions,
    NetWMStrut, NetWMStrutPartial,
    NetWMIconGeometry, NetWMIcon,
    NetWMPid, NetWMHandledIcons, 
    NetWMUserTime, NetWMUserTimeWindow,
    NetWMFrameExtents, NetWMOpaqueRegion,
    NetWMStateFullscreen, 

    NetWMStateAbove, NetWMStateAlwaysOnTop = NetWMStateAbove,

    NetWMStateMaximizedVert, NetWMStateMaximizedHorz,
    NetWMStateBelow, NetWMStateDemandAttention,  NetWMStateSticky,
    NetWMStateShaded, NetWMStateSkipTaskbar, NetWMStateSkipPager,
    NetWMStateModal, NetWMStateHidden, NetWMStateFocused,
    /* action requests */
    NetWMActionMove,
    NetWMActionResize, NetWMActionMaximizeVert, NetWMActionMaximizeHorz,
    NetWMActionFullscreen, NetWMActionChangeDesktop, NetWMActionClose,
    NetWMActionAbove, NetWMActionMinimize, NetWMActionBelow,
    /* actions msg */
    NetWMFullscreen,
    
    /* window types */
    NetWMWindowTypeDesktop, NetWMWindowTypeDock,
    NetWMWindowTypeToolbar, NetWMWindowTypeMenu, 
    NetWMWindowTypeUtility, NetWMWindowTypeSplash, 
    NetWMWindowTypeDropdownMenu, NetWMWindowTypePopupMenu,
    NetWMWindowTypeTooltip, NetWMWindowTypeNotification,
    NetWMWindowTypeCombo, NetWMWindowTypeDnd,
    NetWMWindowTypeDialog, NetWMWindowTypeNormal,

    /* wm protocols */
    NetWMPing, NetWMSyncRequest,
    NetWMFullscreenMonitors,
    /* other */
    NetWMFullPlacement ,NetWMBypassCompositor,
    NetWMWindowOpacity,

    NetUtf8String,
    /* last */
    NetLast,
};
/* default atoms */
enum WMPROTOCOLS { 

    WMName,
    WMIconName,
    WMIconSize,
    WMHints, WMNormalHints,
    WMClass, 
    WMTransientFor,
    WMColormapWindows,
    WMClientMachine,

    WMCommand,

    WMProtocols, 
    WMTakeFocus, WMSaveYourself, WMDeleteWindow, 

    WMState, 
    WMLast 
};



/* Fills wm_atom_return and net_atom_return with every atom part of the X11 standard. + EWMH
 * wm_atom_return are default atoms.
 * net_atom_return are part of the Extended Window Manager Hints. (and some regulars too).
 *
 * Window managers:
 *              Should not use NetLast to apply supported wmhints and netwmhints for windows if they dont support every atom.
 *              Instead they should only use the ones they support.s
 *
 * Usage: Pass in the adress of the returned value(s), using the respective sizes of WMLast, NetLast;
 *        XCBAtom wmatom[WMLast]; XCBAtom netatom[NetLast];
 *        XCBInitAtoms(display, wmatom, netatom);
 *
 * NOTE: No side-effects if wm_atom_return is NULL.
 * NOTE: No side-effects if net_atom_return is NULL.
 * NOTE: XCBInitAtoms() assumes that the space given is enough, i.e. WMLast or NetLast for array size 
 * 
 */
void 
XCBInitNetWMAtomsCookie(
        XCBDisplay *display, 
        XCBCookie *net_cookie_return
        );
void 
XCBInitNetWMAtomsReply(
        XCBDisplay *display, 
        XCBCookie *net_cookies,
        XCBAtom *net_atom_return
        );
void
XCBInitWMAtomsCookie(
        XCBDisplay *display,
        XCBCookie *wm_cookie_return
        );
void
XCBInitWMAtomsReply(
        XCBDisplay *display,
        XCBCookie *wm_cookies,
        XCBAtom *wm_atom_return
        );
int 
XCBGetTextProp(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBAtom atom, 
        char *text, 
        size_t size);

/* Returns a wchar_t * to the name of the window.
 *
 * NOTE: No check for if win exists.
 * NOTE: Data must be freed when finished using.
 *
 * RETURN: wchar_t *
 */
char *
XCBGetWindowName(
        XCBDisplay *display, 
        XCBWindow win
        );

/* Requests for the windows stored pid.
 *
 *
 * RETURN: Cookie to request.
 */
XCBCookie
XCBGetPidCookie(
        XCBDisplay *display,
        XCBWindow win,
        XCBAtom _NET_WM_ICON_ID
        );

/* Gets the pid from the reply, freeing resulting data automatically.
 *
 * RETURN: -1 on Failure.
 * RETURN: int32_t on Success.
 */
int32_t
XCBGetPidReply(
        XCBDisplay *display,
        XCBCookie cookie
        );

#endif
