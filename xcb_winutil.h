#ifndef XCB_WINUTIL_H
#define XCB_WINUTIL_H



#include "xcb_trl.h"



#ifndef False
#define False 0
#endif

#ifndef True
#define True 1
#endif


enum NETWMPROTOCOLS
{
    /* Root window properties */
    NetSupported, NetClientList, 
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
    NetWMStateAlwaysOnTop, NetWMStateStayOnTop,
    NetWMStateMaximizedVert, NetWMStateMaximizedHorz, NetWMStateMinimize, 
    NetWMStateAbove, NetWMStateBelow, NetWMStateDemandAttention,  NetWMStateSticky,
    NetWMStateShaded, NetWMStateSkipTaskbar, NetWMStateSkipPager,
    NetWMStateModal, NetWMStateHidden, NetWMStateFocused,
    /* action requests */
    NetWMActionMove,
    NetWMActionResize, NetWMActionMaximizeVert, NetWMActionMaximizeHorz,
    NetWMActionFullscreen, NetWMActionChangeDesktop, NetWMActionClose,
    NetWMActionAbove, NetWMActionMinimize, NetWMActionBelow,
    /* actions msg */
    NetWMAbove, NetWMBelow, NetWMDemandAttention, NetWMFocused, 
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
    NetWMWindowsOpacity,

    /* last */
    NetLast,
};
/* default atoms */
enum WMPROTOCOLS { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast, };



/* Fills wm_atom_return and net_atom_return with every atom know for X11.
 * wm_atom_return are default atoms.
 * net_atom_return are part of the Extended Window Manager Hints. (and some regulars too)
 *
 * Usage: Pass in the adress of the returned value(s), using the respective sizes of WMLast, NetLast;
 *        XCBAtom wmatom[WMLast]; XCBAtom netatom[NetLast];
 *        XCBInitAtoms(display, &wmatom, &netatom);
 *
 * NOTE: No side-effects if wm_atom_return is NULL.
 * NOTE: No side-effects if net_atom_return is NULL.
 * NOTE: XCBInitAtoms() assumes that the space given is enough, ie WMLast or NetLast for array size 
 * 
 */
void XCBInitAtoms(XCBDisplay *display, XCBAtom *wm_atom_return, XCBAtom *net_atom_return);

#endif
