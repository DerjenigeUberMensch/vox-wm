

#include <stdio.h>
#include <string.h>
#include "xcb_trl.h"
#include "xcb_winutil.h"


void
XCBInitAtoms(XCBDisplay *display, XCBAtom *wm_atom_return, XCBAtom *net_atom_return)
{
    /* wm */
    XCBCookie wmcookies[WMLast];
    XCBCookie netcookies[NetLast];
    if(wm_atom_return)
    {
        wmcookies[WMName] = XCBInternAtomCookie(display, "WM_NAME", False);
        wmcookies[WMIconName] = XCBInternAtomCookie(display, "WM_ICON_NAME", False);
        wmcookies[WMIconSize] = XCBInternAtomCookie(display, "WM_ICON_SIZE", False);
        wmcookies[WMHints] = XCBInternAtomCookie(display, "WM_HINTS", False);
        wmcookies[WMNormalHints] = XCBInternAtomCookie(display, "WM_NORMAL_HINTS", False);
        wmcookies[WMClass] = XCBInternAtomCookie(display, "WM_CLASS", False);
        wmcookies[WMTransientFor] = XCBInternAtomCookie(display, "WM_TRANSIENT_FOR", False);
        wmcookies[WMColormapWindows] = XCBInternAtomCookie(display, "WM_COLORMAP_WINDOWS", False);
        wmcookies[WMClientMachine] = XCBInternAtomCookie(display, "WM_CLIENT_MACHINE", False);
        wmcookies[WMCommand] = XCBInternAtomCookie(display, "WM_COMMAND", False);

        wmcookies[WMTakeFocus] = XCBInternAtomCookie(display, "WM_TAKE_FOCUS", False);
        wmcookies[WMSaveYourself] = XCBInternAtomCookie(display, "WM_SAVE_YOURSELF", False);    /* (deprecated) */
        wmcookies[WMDeleteWindow] = XCBInternAtomCookie(display, "WM_DELETE_WINDOW", False);
        wmcookies[WMProtocols] = XCBInternAtomCookie(display, "WM_PROTOCOLS", False);
        wmcookies[WMState] = XCBInternAtomCookie(display, "WM_STATE", False);

    }

    /* wm state */
    if(net_atom_return)
    {
        netcookies[NetWMState] = XCBInternAtomCookie(display, "_NET_WM_STATE", False);
        netcookies[NetWMStateModal] = XCBInternAtomCookie(display, "_NET_WM_STATE_MODAL", False);
        netcookies[NetWMStateSticky] = XCBInternAtomCookie(display, "_NET_WM_STATE_STICKY", False);
        netcookies[NetWMStateMaximizedVert] = XCBInternAtomCookie(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        netcookies[NetWMStateMaximizedHorz] = XCBInternAtomCookie(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        netcookies[NetWMStateShaded] = XCBInternAtomCookie(display, "_NET_WM_STATE_SHADED", False);
        netcookies[NetWMStateSkipTaskbar] = XCBInternAtomCookie(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
        netcookies[NetWMStateSkipPager] = XCBInternAtomCookie(display, "_NET_WM_STATE_SKIP_PAGER", False);
        netcookies[NetWMStateHidden] = XCBInternAtomCookie(display, "_NET_WM_STATE_HIDDEN", False);
        netcookies[NetWMStateFullscreen] = XCBInternAtomCookie(display, "_NET_WM_STATE_FULLSCREEN", False);
        netcookies[NetWMStateAlwaysOnTop] = netcookies[NetWMStateAbove] = XCBInternAtomCookie(display, "_NET_WM_STATE_ABOVE", False);
        netcookies[NetWMStateBelow] = XCBInternAtomCookie(display, "_NET_WM_STATE_BELOW", False);
        netcookies[NetWMStateDemandAttention] = XCBInternAtomCookie(display, "_NET_WM_STATE_DEMANDS_ATTENTION", False);
        netcookies[NetWMStateFocused] = XCBInternAtomCookie(display, "_NET_WM_STATE_FOCUSED", False);
        netcookies[NetWMStateStayOnTop] = XCBInternAtomCookie(display, "_NET_WM_STATE_STAYS_ON_TOP", False); /* either I have dementia or does this not exists? -dusk */

        /* actions suppoorted */
        netcookies[NetWMActionMove] = XCBInternAtomCookie(display, "_NET_WM_ACTION_MOVE", False);
        netcookies[NetWMActionResize] = XCBInternAtomCookie(display, "_NET_WM_ACTION_RESIZE", False);
        netcookies[NetWMActionMinimize] = XCBInternAtomCookie(display, "_NET_WM_ACTION_MINIMIZE", False);
        netcookies[NetWMActionMaximizeHorz] = XCBInternAtomCookie(display, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
        netcookies[NetWMActionMaximizeVert] = XCBInternAtomCookie(display, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
        netcookies[NetWMActionFullscreen] = XCBInternAtomCookie(display, "_NET_WM_ACTION_FULLSCREEN", False);
        netcookies[NetWMActionChangeDesktop] = XCBInternAtomCookie(display, "_NET_WM_ACTION_CHANGE_DESKTOP", False);
        netcookies[NetWMActionClose] = XCBInternAtomCookie(display, "_NET_WM_ACTION_CLOSE", False);
        netcookies[NetWMActionAbove] = XCBInternAtomCookie(display, "_NET_WM_ACTION_ABOVE", False);
        netcookies[NetWMActionBelow] = XCBInternAtomCookie(display, "_NET_WM_ACTION_BELOW", False);

        /* Root window properties */
        netcookies[NetSupported] = XCBInternAtomCookie(display, "_NET_SUPPORTED", False);
        netcookies[NetClientList] = XCBInternAtomCookie(display, "_NET_CLIENT_LIST", False);
        netcookies[NetNumberOfDesktops] = XCBInternAtomCookie(display, "_NET_NUMBER_OF_DESKTOPS", False);
        netcookies[NetDesktopGeometry] = XCBInternAtomCookie(display, "_NET_DESKTOP_GEOMETRY", False);
        netcookies[NetDesktopViewport] = XCBInternAtomCookie(display, "_NET_DESKTOP_VIEWPORT", False);
        netcookies[NetCurrentDesktop] = XCBInternAtomCookie(display, "_NET_CURRENT_DESKTOP", False);
        netcookies[NetDesktopNames] = XCBInternAtomCookie(display, "_NET_DESKTOP_NAMES", False);
        netcookies[NetWorkarea] = XCBInternAtomCookie(display, "_NET_WORKAREA", False);
        netcookies[NetSupportingWMCheck] = XCBInternAtomCookie(display, "_NET_SUPPORTING_WM_CHECK", False);
        netcookies[NetVirtualRoots] = XCBInternAtomCookie(display, "_NET_VIRTUAL_ROOTS", False);
        netcookies[NetDesktopLayout] = XCBInternAtomCookie(display, "_NET_DESKTOP_LAYOUT", False);
        netcookies[NetShowingDesktop] = XCBInternAtomCookie(display, "_NET_SHOWING_DESKTOP", False);

        /* other root messages */
        netcookies[NetCloseWindow] = XCBInternAtomCookie(display, "_NET_CLOSE_WINDOW", False);
        netcookies[NetMoveResizeWindow] = XCBInternAtomCookie(display, "_NET_MOVERESIZE_WINDOW", False);
        netcookies[NetMoveResize] = XCBInternAtomCookie(display, "_NET_WM_MOVERESIZE", False);
        netcookies[NetRestackWindow] = XCBInternAtomCookie(display, "_NET_RESTACK_WINDOW", False);
        netcookies[NetRequestFrameExtents] = XCBInternAtomCookie(display, "_NET_REQUEST_FRAME_EXTENTS", False);
        netcookies[NetActiveWindow] = XCBInternAtomCookie(display, "_NET_ACTIVE_WINDOW", False);

        /* application win properties */
        netcookies[NetWMName] = XCBInternAtomCookie(display, "_NET_WM_NAME", False);
        netcookies[NetWMVisibleName] = XCBInternAtomCookie(display, "_NET_WM_VISIBLE_NAME", False);
        netcookies[NetWMIconName] = XCBInternAtomCookie(display, "_NET_WM_ICON_NAME", False);
        netcookies[NetWMVisibleIconName] = XCBInternAtomCookie(display, "_NET_WM_VISIBLE_ICON_NAME", False);
        netcookies[NetWMDesktop] = XCBInternAtomCookie(display, "_NET_WM_DESKTOP", False);
        netcookies[NetWMAllowedActions] = XCBInternAtomCookie(display, "_NET_WM_ALLOWED_ACTIONS", False);
        netcookies[NetWMStrut] = XCBInternAtomCookie(display, "_NET_WM_STRUT", False);
        netcookies[NetWMStrutPartial] = XCBInternAtomCookie(display, "_NET_WM_STRUT_PARTIAL", False);
        netcookies[NetWMIconGeometry] = XCBInternAtomCookie(display, "_NET_WM_ICON_GEOMETRY", False);
        netcookies[NetWMIcon] = XCBInternAtomCookie(display, "_NET_WM_ICON", False);
        netcookies[NetWMPid] = XCBInternAtomCookie(display, "_NET_WM_PID", False);
        netcookies[NetWMHandledIcons] = XCBInternAtomCookie(display, "_NET_WM_HANDLED_ICONS", False);
        netcookies[NetWMFrameExtents] = XCBInternAtomCookie(display, "_NET_FRAME_EXTENTS", False);
        netcookies[NetWMOpaqueRegion] = XCBInternAtomCookie(display, "_NET_WM_OPAQUE_REGION", False);
        netcookies[NetWMBypassCompositor] = XCBInternAtomCookie(display, "_NET_WM_BYPASS_COMPOSITOR", False);
        //netcookies[NetWMMinimize] = XCBInternAtomCookie(display, "_NET_WM_MINIMIZE", False);
        /* window types */
        netcookies[NetWMWindowType] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE", False);
        netcookies[NetWMWindowTypeDesktop] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
        netcookies[NetWMWindowTypeDock] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
        netcookies[NetWMWindowTypeToolbar] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
        netcookies[NetWMWindowTypeMenu] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_MENU", False);
        netcookies[NetWMWindowTypeUtility] = XCBInternAtomCookie(display, "_NET_WMWINDOW_TYPE_UTILITY", False);
        netcookies[NetWMWindowTypeSplash] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
        netcookies[NetWMWindowTypeDialog] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        netcookies[NetWMWindowTypeDropdownMenu] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", False);
        netcookies[NetWMWindowTypePopupMenu] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
        netcookies[NetWMWindowTypeTooltip] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_TOOLTIP", False);
        netcookies[NetWMWindowTypeNotification] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
        netcookies[NetWMWindowTypeCombo] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_COMBO", False);
        netcookies[NetWMWindowTypeDnd] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_DND", False);
        netcookies[NetWMWindowTypeNormal] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
        /* Window manager protocols */
        netcookies[NetWMPing] = XCBInternAtomCookie(display, "_NET_WM_PING", False);
        netcookies[NetWMSyncRequest] = XCBInternAtomCookie(display, "_NET_WM_SYNC_REQUEST", False);
        netcookies[NetWMFullscreenMonitors] = XCBInternAtomCookie(display, "_NET_WM_FULLSCREEN_MONITORS", False);
        netcookies[NetWMUserTime] = XCBInternAtomCookie(display, "_NET_WM_USER_TIME", False);
        netcookies[NetWMUserTimeWindow] = XCBInternAtomCookie(display, "_NET_WM_USER_TIME_WINDOW", False);

        /* stuff */
        netcookies[NetWMFullscreen] = XCBInternAtomCookie(display, "_NET_WM_FULLSCREEN", False);
        netcookies[NetWMAbove] = XCBInternAtomCookie(display, "_NET_WM_ABOVE", False);

        /* other */
        netcookies[NetWMFullPlacement] = XCBInternAtomCookie(display, "_NET_WM_FULL_PLACEMENT", False);
        netcookies[NetWMWindowsOpacity] = XCBInternAtomCookie(display, "_NET_WM_WINDOW_OPACITY", False);
    }

    if(wm_atom_return)
    {
        wm_atom_return[WMName] = XCBInternAtomReply(display, wmcookies[WMName]);
        wm_atom_return[WMIconName] = XCBInternAtomReply(display, wmcookies[WMIconName]);
        wm_atom_return[WMIconSize] = XCBInternAtomReply(display,wmcookies[WMIconSize]);
        wm_atom_return[WMHints] = XCBInternAtomReply(display, wmcookies[WMHints]);
        wm_atom_return[WMNormalHints] = XCBInternAtomReply(display, wmcookies[WMNormalHints]);
        wm_atom_return[WMClass] = XCBInternAtomReply(display, wmcookies[WMClass]);
        wm_atom_return[WMTransientFor] = XCBInternAtomReply(display, wmcookies[WMTransientFor]);
        wm_atom_return[WMColormapWindows] = XCBInternAtomReply(display, wmcookies[WMColormapWindows]);
        wm_atom_return[WMClientMachine] = XCBInternAtomReply(display, wmcookies[WMClientMachine]);
        wm_atom_return[WMCommand] = XCBInternAtomReply(display, wmcookies[WMCommand]);

        wm_atom_return[WMProtocols] = XCBInternAtomReply(display, wmcookies[WMProtocols]);
        wm_atom_return[WMDeleteWindow] = XCBInternAtomReply(display, wmcookies[WMDeleteWindow]);
        wm_atom_return[WMState] = XCBInternAtomReply(display, wmcookies[WMState]);
        wm_atom_return[WMTakeFocus] = XCBInternAtomReply(display, wmcookies[WMTakeFocus]);
    }
    if(net_atom_return)
    {
        net_atom_return[NetWMState] = XCBInternAtomReply(display, netcookies[NetWMState]);
        net_atom_return[NetWMStateModal] = XCBInternAtomReply(display, netcookies[NetWMStateModal]);
        net_atom_return[NetWMStateSticky] = XCBInternAtomReply(display, netcookies[NetWMStateSticky]);
        net_atom_return[NetWMStateMaximizedVert] = XCBInternAtomReply(display, netcookies[NetWMStateMaximizedVert]);
        net_atom_return[NetWMStateMaximizedHorz] = XCBInternAtomReply(display, netcookies[NetWMStateMaximizedHorz]);
        net_atom_return[NetWMStateShaded] = XCBInternAtomReply(display, netcookies[NetWMStateShaded]);
        net_atom_return[NetWMStateSkipTaskbar] = XCBInternAtomReply(display, netcookies[NetWMStateSkipTaskbar]);
        net_atom_return[NetWMStateSkipPager] = XCBInternAtomReply(display, netcookies[NetWMStateSkipPager]);
        net_atom_return[NetWMStateHidden] = XCBInternAtomReply(display, netcookies[NetWMStateHidden]);
        net_atom_return[NetWMStateFullscreen] = XCBInternAtomReply(display, netcookies[NetWMStateFullscreen]);
        net_atom_return[NetWMStateAlwaysOnTop] = XCBInternAtomReply(display, netcookies[NetWMStateAlwaysOnTop]);
        net_atom_return[NetWMStateAbove] = net_atom_return[NetWMStateAlwaysOnTop];
        net_atom_return[NetWMStateBelow] = XCBInternAtomReply(display, netcookies[NetWMStateBelow]);
        net_atom_return[NetWMStateDemandAttention] = XCBInternAtomReply(display, netcookies[NetWMStateDemandAttention]);
        net_atom_return[NetWMStateFocused] = XCBInternAtomReply(display, netcookies[NetWMStateFocused]);
        net_atom_return[NetWMStateStayOnTop] = XCBInternAtomReply(display, netcookies[NetWMStateStayOnTop]);

        /* actions suppoorted */
        net_atom_return[NetWMActionMove] = XCBInternAtomReply(display, netcookies[NetWMActionMove]);
        net_atom_return[NetWMActionResize] = XCBInternAtomReply(display, netcookies[NetWMActionResize]);
        net_atom_return[NetWMActionMinimize] = XCBInternAtomReply(display, netcookies[NetWMActionMinimize]);
        net_atom_return[NetWMActionMaximizeHorz] = XCBInternAtomReply(display, netcookies[NetWMActionMaximizeHorz]);
        net_atom_return[NetWMActionMaximizeVert] = XCBInternAtomReply(display, netcookies[NetWMActionMaximizeVert]);
        net_atom_return[NetWMActionFullscreen] = XCBInternAtomReply(display, netcookies[NetWMActionFullscreen]);
        net_atom_return[NetWMActionChangeDesktop] = XCBInternAtomReply(display, netcookies[NetWMActionChangeDesktop]);
        net_atom_return[NetWMActionClose] = XCBInternAtomReply(display, netcookies[NetWMActionClose]);
        net_atom_return[NetWMActionAbove] = XCBInternAtomReply(display, netcookies[NetWMActionAbove]);
        net_atom_return[NetWMActionBelow] = XCBInternAtomReply(display, netcookies[NetWMActionBelow]);

        /* Root window properties */
        net_atom_return[NetSupported] = XCBInternAtomReply(display, netcookies[NetSupported]);
        net_atom_return[NetClientList] = XCBInternAtomReply(display, netcookies[NetClientList]);
        net_atom_return[NetNumberOfDesktops] = XCBInternAtomReply(display, netcookies[NetNumberOfDesktops]);
        net_atom_return[NetDesktopGeometry] = XCBInternAtomReply(display, netcookies[NetDesktopGeometry]);
        net_atom_return[NetDesktopViewport] = XCBInternAtomReply(display, netcookies[NetDesktopViewport]);
        net_atom_return[NetCurrentDesktop] = XCBInternAtomReply(display, netcookies[NetCurrentDesktop]);
        net_atom_return[NetDesktopNames] = XCBInternAtomReply(display, netcookies[NetDesktopNames]);
        net_atom_return[NetWorkarea] = XCBInternAtomReply(display, netcookies[NetWorkarea]);
        net_atom_return[NetSupportingWMCheck] = XCBInternAtomReply(display, netcookies[NetSupportingWMCheck]);
        net_atom_return[NetVirtualRoots] = XCBInternAtomReply(display, netcookies[NetVirtualRoots]);
        net_atom_return[NetDesktopLayout] = XCBInternAtomReply(display, netcookies[NetDesktopLayout]);
        net_atom_return[NetShowingDesktop] = XCBInternAtomReply(display, netcookies[NetShowingDesktop]);

        /* other root messages */
        net_atom_return[NetCloseWindow] = XCBInternAtomReply(display, netcookies[NetCloseWindow]);
        net_atom_return[NetMoveResizeWindow] = XCBInternAtomReply(display, netcookies[NetMoveResizeWindow]);
        net_atom_return[NetMoveResize] = XCBInternAtomReply(display, netcookies[NetMoveResize]);
        net_atom_return[NetRestackWindow] = XCBInternAtomReply(display, netcookies[NetRestackWindow]);
        net_atom_return[NetRequestFrameExtents] = XCBInternAtomReply(display, netcookies[NetRequestFrameExtents]);
        net_atom_return[NetActiveWindow] = XCBInternAtomReply(display, netcookies[NetActiveWindow]);

        /* application win properties */
        net_atom_return[NetWMName] = XCBInternAtomReply(display, netcookies[NetWMName]);
        net_atom_return[NetWMVisibleName] = XCBInternAtomReply(display, netcookies[NetWMVisibleName]);
        net_atom_return[NetWMIconName] = XCBInternAtomReply(display, netcookies[NetWMIconName]);
        net_atom_return[NetWMVisibleIconName] = XCBInternAtomReply(display, netcookies[NetWMVisibleIconName]);
        net_atom_return[NetWMDesktop] = XCBInternAtomReply(display, netcookies[NetWMDesktop]);
        net_atom_return[NetWMAllowedActions] = XCBInternAtomReply(display, netcookies[NetWMAllowedActions]);
        net_atom_return[NetWMStrut] = XCBInternAtomReply(display, netcookies[NetWMStrut]);
        net_atom_return[NetWMStrutPartial] = XCBInternAtomReply(display, netcookies[NetWMStrutPartial]);
        net_atom_return[NetWMIconGeometry] = XCBInternAtomReply(display, netcookies[NetWMIconGeometry]);
        net_atom_return[NetWMIcon] = XCBInternAtomReply(display, netcookies[NetWMIcon]);
        net_atom_return[NetWMPid] = XCBInternAtomReply(display, netcookies[NetWMPid]);
        net_atom_return[NetWMHandledIcons] = XCBInternAtomReply(display, netcookies[NetWMHandledIcons]);
        net_atom_return[NetWMFrameExtents] = XCBInternAtomReply(display, netcookies[NetWMFrameExtents]);
        net_atom_return[NetWMOpaqueRegion] = XCBInternAtomReply(display, netcookies[NetWMOpaqueRegion]);
        net_atom_return[NetWMBypassCompositor] = XCBInternAtomReply(display, netcookies[NetWMBypassCompositor]);
        //net_atom_return[] = XCBInternAtomReply(display, netcookies[NetWMMinimize]_NET_WM_MINIMIZE", False);
        /* window types */
        net_atom_return[NetWMWindowType] = XCBInternAtomReply(display, netcookies[NetWMWindowType]);
        net_atom_return[NetWMWindowTypeDesktop] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeDesktop]);
        net_atom_return[NetWMWindowTypeDock] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeDock]);
        net_atom_return[NetWMWindowTypeToolbar] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeToolbar]);
        net_atom_return[NetWMWindowTypeMenu] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeMenu]);
        net_atom_return[NetWMWindowTypeUtility] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeUtility]);
        net_atom_return[NetWMWindowTypeSplash] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeSplash]);
        net_atom_return[NetWMWindowTypeDialog] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeDialog]);
        net_atom_return[NetWMWindowTypeDropdownMenu] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeDropdownMenu]);
        net_atom_return[NetWMWindowTypePopupMenu] = XCBInternAtomReply(display, netcookies[NetWMWindowTypePopupMenu]);
        net_atom_return[NetWMWindowTypeTooltip] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeTooltip]);
        net_atom_return[NetWMWindowTypeNotification] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeNotification]);
        net_atom_return[NetWMWindowTypeCombo] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeCombo]);
        net_atom_return[NetWMWindowTypeDnd] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeDnd]);
        net_atom_return[NetWMWindowTypeNormal] = XCBInternAtomReply(display, netcookies[NetWMWindowTypeNormal]);
        /* Window manager protocols */
        net_atom_return[NetWMPing] = XCBInternAtomReply(display, netcookies[NetWMPing]);
        net_atom_return[NetWMSyncRequest] = XCBInternAtomReply(display, netcookies[NetWMSyncRequest]);
        net_atom_return[NetWMFullscreenMonitors] = XCBInternAtomReply(display, netcookies[NetWMFullscreenMonitors]);
        net_atom_return[NetWMUserTime] = XCBInternAtomReply(display, netcookies[NetWMUserTime]);
        net_atom_return[NetWMUserTimeWindow] = XCBInternAtomReply(display, netcookies[NetWMUserTimeWindow]);

        /* stuff */
        net_atom_return[NetWMFullscreen] = XCBInternAtomReply(display, netcookies[NetWMFullscreen]);
        net_atom_return[NetWMAbove] = XCBInternAtomReply(display, netcookies[NetWMAbove]);


        /* other */
        net_atom_return[NetWMFullPlacement] = XCBInternAtomReply(display, netcookies[NetWMFullPlacement]);
        net_atom_return[NetWMWindowsOpacity] = XCBInternAtomReply(display, netcookies[NetWMWindowsOpacity]);
    }
}


int 
XCBGetTextProp(
        XCBDisplay *display, 
        XCBWindow window, 
        XCBAtom atom, 
        char *text, 
        size_t size)
{
	char **list = NULL;
	int n;
    XCBCookie cookie;
	XCBTextProperty name;


	if (!text || size == 0)
		return 0;

	text[0] = '\0';

    cookie = XCBGetTextPropertyCookie(display, window, atom);
    if(!XCBGetTextPropertyReply(display, cookie, &name) || !name.name_len)
    {   return 0;
    }
                        /* XA_STRING I think */
    if (name.encoding == XCB_ATOM_STRING) 
    {   strncpy(text, (char *)name.name, size - 1);
    }
    else
    {
        /* this is questionably hard to replace src/xlibi18n/lcWrap.c 
         * XSupportsLocale() is also in here so might was well get that too
         */
        /*
           XTextProperty prop;
           prop.encoding = name->encoding;
           prop.format = name->format;
           prop.value = (unsigned char *)name->name;
           prop.nitems = name->name_len;

           if (XmbTextPropertyToTextList(Xdpy, &prop, &list, &n) >= Success && n > 0 && *list) 
           {
           strncpy(text, *list, size - 1);
           XFreeStringList(list);
           }
           */
    } 
	text[size - 1] = '\0';
    XCBFreeTextProperty(&name);
	return 1;
}

int
XCBGetWindowName(
        XCBDisplay *display, 
        XCBWindow win, 
        XCBAtom _NET_WM_NAME,
        char **name, 
        uint32_t name_len)
{
    return 1;
}
