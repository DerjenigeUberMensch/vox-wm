

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include "xcb_trl.h"
#include "xcb_winutil.h"

static const char *const WM_ATOM_NAMES[WMLast]= 
{
    [WMName]              = "WM_NAME",
    [WMIconName]          = "WM_ICON_NAME",
    [WMIconSize]          = "WM_ICON_SIZE",
    [WMHints]             = "WM_HINTS",
    [WMNormalHints]       = "WM_NORMAL_HINTS",
    [WMClass]             = "WM_CLASS",
    [WMTransientFor]      = "WM_TRANSIENT_FOR",
    [WMColormapWindows]   = "WM_COLORMAP_WINDOWS",
    [WMClientMachine]     = "WM_CLIENT_MACHINE",
    [WMCommand]           = "WM_COMMAND",
    [WMTakeFocus]         = "WM_TAKE_FOCUS",
    [WMSaveYourself]      = "WM_SAVE_YOURSELF", /* (deprecated) */
    [WMDeleteWindow]      = "WM_DELETE_WINDOW", 
    [WMProtocols]         = "WM_PROTOCOLS",
    [WMState]             = "WM_STATE"
};

static const unsigned int WM_ATOM_LEN[WMLast] = 
{
    [WMName]              = sizeof("WM_NAME") - 1,
    [WMIconName]          = sizeof("WM_ICON_NAME") - 1,
    [WMIconSize]          = sizeof("WM_ICON_SIZE") - 1,
    [WMHints]             = sizeof("WM_HINTS") - 1,
    [WMNormalHints]       = sizeof("WM_NORMAL_HINTS") - 1,
    [WMClass]             = sizeof("WM_CLASS") - 1,
    [WMTransientFor]      = sizeof("WM_TRANSIENT_FOR") - 1,
    [WMColormapWindows]   = sizeof("WM_COLORMAP_WINDOWS") - 1,
    [WMClientMachine]     = sizeof("WM_CLIENT_MACHINE") - 1,
    [WMCommand]           = sizeof("WM_COMMAND") - 1,
    [WMTakeFocus]         = sizeof("WM_TAKE_FOCUS") - 1,
    [WMSaveYourself]      = sizeof("WM_SAVE_YOURSELF") - 1, /* (deprecated) */
    [WMDeleteWindow]      = sizeof("WM_DELETE_WINDOW") - 1,
    [WMProtocols]         = sizeof("WM_PROTOCOLS") - 1,
    [WMState]             = sizeof("WM_STATE") - 1
};

static const char *const NET_WM_ATOM_NAMES[NetLast] =
{
    [NetSupported]               = "_NET_SUPPORTED",
    [NetClientList]              = "_NET_CLIENT_LIST",
    [NetNumberOfDesktops]        = "_NET_NUMBER_OF_DESKTOPS",
    [NetDesktopGeometry]         = "_NET_DESKTOP_GEOMETRY",
    [NetDesktopViewport]         = "_NET_DESKTOP_VIEWPORT",
    [NetCurrentDesktop]          = "_NET_CURRENT_DESKTOP",
    [NetDesktopNames]            = "_NET_DESKTOP_NAMES",
    [NetActiveWindow]            = "_NET_ACTIVE_WINDOW",
    [NetWorkarea]                = "_NET_WORKAREA",
    [NetSupportingWMCheck]       = "_NET_SUPPORTING_WM_CHECK",
    [NetVirtualRoots]            = "_NET_VIRTUAL_ROOTS",
    [NetDesktopLayout]           = "_NET_DESKTOP_LAYOUT",
    [NetShowingDesktop]          = "_NET_SHOWING_DESKTOP",
    [NetCloseWindow]             = "_NET_CLOSE_WINDOW",
    [NetMoveResizeWindow]        = "_NET_MOVERESIZE_WINDOW",
    [NetMoveResize]              = "_NET_WM_MOVERESIZE",
    [NetRestackWindow]           = "_NET_RESTACK_WINDOW",
    [NetRequestFrameExtents]     = "_NET_REQUEST_FRAME_EXTENTS",
    [NetWMName]                  = "_NET_WM_NAME",
    [NetWMVisibleName]           = "_NET_WM_VISIBLE_NAME",
    [NetWMIconName]              = "_NET_WM_ICON_NAME",
    [NetWMVisibleIconName]       = "_NET_WM_VISIBLE_ICON_NAME",
    [NetWMDesktop]               = "_NET_WM_DESKTOP",
    [NetWMWindowType]            = "_NET_WM_WINDOW_TYPE",
    [NetWMState]                 = "_NET_WM_STATE",
    [NetWMAllowedActions]        = "_NET_WM_ALLOWED_ACTIONS",
    [NetWMStrut]                 = "_NET_WM_STRUT",
    [NetWMStrutPartial]          = "_NET_WM_STRUT_PARTIAL",
    [NetWMIconGeometry]          = "_NET_WM_ICON_GEOMETRY",
    [NetWMIcon]                  = "_NET_WM_ICON",
    [NetWMPid]                   = "_NET_WM_PID",
    [NetWMHandledIcons]          = "_NET_WM_HANDLED_ICONS",
    [NetWMUserTime]              = "_NET_WM_USER_TIME",
    [NetWMUserTimeWindow]        = "_NET_WM_USER_TIME_WINDOW",
    [NetWMFrameExtents]          = "_NET_FRAME_EXTENTS",
    [NetWMOpaqueRegion]          = "_NET_WM_OPAQUE_REGION",
    [NetWMStateFullscreen]       = "_NET_WM_STATE_FULLSCREEN",
    [NetWMStateAbove]            = "_NET_WM_STATE_ABOVE",
    [NetWMStateMaximizedVert]    = "_NET_WM_STATE_MAXIMIZED_VERT",
    [NetWMStateMaximizedHorz]    = "_NET_WM_STATE_MAXIMIZED_HORZ",
    [NetWMStateBelow]            = "_NET_WM_STATE_BELOW",
    [NetWMStateDemandAttention]  = "_NET_WM_STATE_DEMANDS_ATTENTION",
    [NetWMStateSticky]           = "_NET_WM_STATE_STICKY",
    [NetWMStateShaded]           = "_NET_WM_STATE_SHADED",
    [NetWMStateSkipTaskbar]      = "_NET_WM_STATE_SKIP_TASKBAR",
    [NetWMStateSkipPager]        = "_NET_WM_STATE_SKIP_PAGER",
    [NetWMStateModal]            = "_NET_WM_STATE_MODAL",
    [NetWMStateHidden]           = "_NET_WM_STATE_HIDDEN",
    [NetWMStateFocused]          = "_NET_WM_STATE_FOCUSED",
    [NetWMActionMove]            = "_NET_WM_ACTION_MOVE",
    [NetWMActionResize]          = "_NET_WM_ACTION_RESIZE",
    [NetWMActionMaximizeVert]    = "_NET_WM_ACTION_MAXIMIZE_VERT",
    [NetWMActionMaximizeHorz]    = "_NET_WM_ACTION_MAXIMIZE_HORZ",
    [NetWMActionFullscreen]      = "_NET_WM_ACTION_FULLSCREEN",
    [NetWMActionChangeDesktop]   = "_NET_WM_ACTION_CHANGE_DESKTOP",
    [NetWMActionClose]           = "_NET_WM_ACTION_CLOSE",
    [NetWMActionAbove]           = "_NET_WM_ACTION_ABOVE",
    [NetWMActionMinimize]        = "_NET_WM_ACTION_MINIMIZE",
    [NetWMActionBelow]           = "_NET_WM_ACTION_BELOW",
    [NetWMFullscreen]            = "_NET_WM_FULLSCREEN",
    [NetWMWindowTypeDesktop]     = "_NET_WM_WINDOW_TYPE_DESKTOP",
    [NetWMWindowTypeDock]        = "_NET_WM_WINDOW_TYPE_DOCK",
    [NetWMWindowTypeToolbar]     = "_NET_WM_WINDOW_TYPE_TOOLBAR",
    [NetWMWindowTypeMenu]        = "_NET_WM_WINDOW_TYPE_MENU",
    [NetWMWindowTypeUtility]     = "_NET_WM_WINDOW_TYPE_UTILITY",
    [NetWMWindowTypeSplash]      = "_NET_WM_WINDOW_TYPE_SPLASH",
    [NetWMWindowTypeDropdownMenu]= "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
    [NetWMWindowTypePopupMenu]   = "_NET_WM_WINDOW_TYPE_POPUP_MENU",
    [NetWMWindowTypeTooltip]     = "_NET_WM_WINDOW_TYPE_TOOLTIP",
    [NetWMWindowTypeNotification]= "_NET_WM_WINDOW_TYPE_NOTIFICATION",
    [NetWMWindowTypeCombo]       = "_NET_WM_WINDOW_TYPE_COMBO",
    [NetWMWindowTypeDnd]         = "_NET_WM_WINDOW_TYPE_DND",
    [NetWMWindowTypeDialog]      = "_NET_WM_WINDOW_TYPE_DIALOG",
    [NetWMWindowTypeNormal]      = "_NET_WM_WINDOW_TYPE_NORMAL",
    [NetWMPing]                  = "_NET_WM_PING",
    [NetWMSyncRequest]           = "_NET_WM_SYNC_REQUEST",
    [NetWMFullscreenMonitors]    = "_NET_WM_FULLSCREEN_MONITORS",
    [NetWMFullPlacement]         = "_NET_WM_FULL_PLACEMENT",
    [NetWMBypassCompositor]      = "_NET_WM_BYPASS_COMPOSITOR",
    [NetWMWindowOpacity]        = "_NET_WM_WINDOW_OPACITY",
    [NetUtf8String]              = "UTF8_STRING",
};

static const unsigned int NET_WM_ATOM_LEN[NetLast] = 
{
    [NetSupported]               = sizeof("_NET_SUPPORTED") - 1,
    [NetClientList]              = sizeof("_NET_CLIENT_LIST") - 1,
    [NetNumberOfDesktops]        = sizeof("_NET_NUMBER_OF_DESKTOPS") - 1,
    [NetDesktopGeometry]         = sizeof("_NET_DESKTOP_GEOMETRY") - 1,
    [NetDesktopViewport]         = sizeof("_NET_DESKTOP_VIEWPORT") - 1,
    [NetCurrentDesktop]          = sizeof("_NET_CURRENT_DESKTOP") - 1,
    [NetDesktopNames]            = sizeof("_NET_DESKTOP_NAMES") - 1,
    [NetActiveWindow]            = sizeof("_NET_ACTIVE_WINDOW") - 1,
    [NetWorkarea]                = sizeof("_NET_WORKAREA") - 1,
    [NetSupportingWMCheck]       = sizeof("_NET_SUPPORTING_WM_CHECK") - 1,
    [NetVirtualRoots]            = sizeof("_NET_VIRTUAL_ROOTS") - 1,
    [NetDesktopLayout]           = sizeof("_NET_DESKTOP_LAYOUT") - 1,
    [NetShowingDesktop]          = sizeof("_NET_SHOWING_DESKTOP") - 1,
    [NetCloseWindow]             = sizeof("_NET_CLOSE_WINDOW") - 1,
    [NetMoveResizeWindow]        = sizeof("_NET_MOVERESIZE_WINDOW") - 1,
    [NetMoveResize]              = sizeof("_NET_WM_MOVERESIZE") - 1,
    [NetRestackWindow]           = sizeof("_NET_RESTACK_WINDOW") - 1,
    [NetRequestFrameExtents]     = sizeof("_NET_REQUEST_FRAME_EXTENTS") - 1,
    [NetWMName]                  = sizeof("_NET_WM_NAME") - 1,
    [NetWMVisibleName]           = sizeof("_NET_WM_VISIBLE_NAME") - 1,
    [NetWMIconName]              = sizeof("_NET_WM_ICON_NAME") - 1,
    [NetWMVisibleIconName]       = sizeof("_NET_WM_VISIBLE_ICON_NAME") - 1,
    [NetWMDesktop]               = sizeof("_NET_WM_DESKTOP") - 1,
    [NetWMWindowType]            = sizeof("_NET_WM_WINDOW_TYPE") - 1,
    [NetWMState]                 = sizeof("_NET_WM_STATE") - 1,
    [NetWMAllowedActions]        = sizeof("_NET_WM_ALLOWED_ACTIONS") - 1,
    [NetWMStrut]                 = sizeof("_NET_WM_STRUT") - 1,
    [NetWMStrutPartial]          = sizeof("_NET_WM_STRUT_PARTIAL") - 1,
    [NetWMIconGeometry]          = sizeof("_NET_WM_ICON_GEOMETRY") - 1,
    [NetWMIcon]                  = sizeof("_NET_WM_ICON") - 1,
    [NetWMPid]                   = sizeof("_NET_WM_PID") - 1,
    [NetWMHandledIcons]          = sizeof("_NET_WM_HANDLED_ICONS") - 1,
    [NetWMUserTime]              = sizeof("_NET_WM_USER_TIME") - 1,
    [NetWMUserTimeWindow]        = sizeof("_NET_WM_USER_TIME_WINDOW") - 1,
    [NetWMFrameExtents]          = sizeof("_NET_FRAME_EXTENTS") - 1,
    [NetWMOpaqueRegion]          = sizeof("_NET_WM_OPAQUE_REGION") - 1,
    [NetWMStateFullscreen]       = sizeof("_NET_WM_STATE_FULLSCREEN") - 1,
    [NetWMStateAbove]            = sizeof("_NET_WM_STATE_ABOVE") - 1,
    [NetWMStateMaximizedVert]    = sizeof("_NET_WM_STATE_MAXIMIZED_VERT") - 1,
    [NetWMStateMaximizedHorz]    = sizeof("_NET_WM_STATE_MAXIMIZED_HORZ") - 1,
    [NetWMStateBelow]            = sizeof("_NET_WM_STATE_BELOW") - 1,
    [NetWMStateDemandAttention]  = sizeof("_NET_WM_STATE_DEMANDS_ATTENTION") - 1,
    [NetWMStateSticky]           = sizeof("_NET_WM_STATE_STICKY") - 1,
    [NetWMStateShaded]           = sizeof("_NET_WM_STATE_SHADED") - 1,
    [NetWMStateSkipTaskbar]      = sizeof("_NET_WM_STATE_SKIP_TASKBAR") - 1,
    [NetWMStateSkipPager]        = sizeof("_NET_WM_STATE_SKIP_PAGER") - 1,
    [NetWMStateModal]            = sizeof("_NET_WM_STATE_MODAL") - 1,
    [NetWMStateHidden]           = sizeof("_NET_WM_STATE_HIDDEN") - 1,
    [NetWMStateFocused]          = sizeof("_NET_WM_STATE_FOCUSED") - 1,
    [NetWMActionMove]            = sizeof("_NET_WM_ACTION_MOVE") - 1,
    [NetWMActionResize]          = sizeof("_NET_WM_ACTION_RESIZE") - 1,
    [NetWMActionMaximizeVert]    = sizeof("_NET_WM_ACTION_MAXIMIZE_VERT") - 1,
    [NetWMActionMaximizeHorz]    = sizeof("_NET_WM_ACTION_MAXIMIZE_HORZ") - 1,
    [NetWMActionFullscreen]      = sizeof("_NET_WM_ACTION_FULLSCREEN") - 1,
    [NetWMActionChangeDesktop]   = sizeof("_NET_WM_ACTION_CHANGE_DESKTOP") - 1,
    [NetWMActionClose]           = sizeof("_NET_WM_ACTION_CLOSE") - 1,
    [NetWMActionAbove]           = sizeof("_NET_WM_ACTION_ABOVE") - 1,
    [NetWMActionMinimize]        = sizeof("_NET_WM_ACTION_MINIMIZE") - 1,
    [NetWMActionBelow]           = sizeof("_NET_WM_ACTION_BELOW") - 1,
    [NetWMFullscreen]            = sizeof("_NET_WM_FULLSCREEN") - 1,
    [NetWMWindowTypeDesktop]     = sizeof("_NET_WM_WINDOW_TYPE_DESKTOP") - 1,
    [NetWMWindowTypeDock]        = sizeof("_NET_WM_WINDOW_TYPE_DOCK") - 1,
    [NetWMWindowTypeToolbar]     = sizeof("_NET_WM_WINDOW_TYPE_TOOLBAR") - 1,
    [NetWMWindowTypeMenu]        = sizeof("_NET_WM_WINDOW_TYPE_MENU") - 1,
    [NetWMWindowTypeUtility]     = sizeof("_NET_WM_WINDOW_TYPE_UTILITY") - 1,
    [NetWMWindowTypeSplash]      = sizeof("_NET_WM_WINDOW_TYPE_SPLASH") - 1,
    [NetWMWindowTypeDropdownMenu]= sizeof("_NET_WM_WINDOW_TYPE_DROPDOWN_MENU") - 1,
    [NetWMWindowTypePopupMenu]   = sizeof("_NET_WM_WINDOW_TYPE_POPUP_MENU") - 1,
    [NetWMWindowTypeTooltip]     = sizeof("_NET_WM_WINDOW_TYPE_TOOLTIP") - 1,
    [NetWMWindowTypeNotification]= sizeof("_NET_WM_WINDOW_TYPE_NOTIFICATION") - 1,
    [NetWMWindowTypeCombo]       = sizeof("_NET_WM_WINDOW_TYPE_COMBO") - 1,
    [NetWMWindowTypeDnd]         = sizeof("_NET_WM_WINDOW_TYPE_DND") - 1,
    [NetWMWindowTypeDialog]      = sizeof("_NET_WM_WINDOW_TYPE_DIALOG") - 1,
    [NetWMWindowTypeNormal]      = sizeof("_NET_WM_WINDOW_TYPE_NORMAL") - 1,
    [NetWMPing]                  = sizeof("_NET_WM_PING") - 1,
    [NetWMSyncRequest]           = sizeof("_NET_WM_SYNC_REQUEST") - 1,
    [NetWMFullscreenMonitors]    = sizeof("_NET_WM_FULLSCREEN_MONITORS") - 1,
    [NetWMFullPlacement]         = sizeof("_NET_WM_FULL_PLACEMENT") - 1,
    [NetWMBypassCompositor]      = sizeof("_NET_WM_BYPASS_COMPOSITOR") - 1,
    [NetWMWindowOpacity]         = sizeof("_NET_WM_WINDOW_OPACITY") - 1,
    [NetUtf8String]              = sizeof("UTF8_STRING") - 1,
};

void
XCBInitWMAtomsCookie(
        XCBDisplay *display, 
        XCBCookie *net_cookie_return
        )
{
    unsigned int i;
    for(i = 0; i < WMLast; ++i)
    {   net_cookie_return[i] = (XCBCookie) { .sequence = xcb_intern_atom(display, False, WM_ATOM_LEN[i], WM_ATOM_NAMES[i]).sequence };
    }
}

void
XCBInitWMAtomsReply(
        XCBDisplay *display,
        XCBCookie *cookies,
        XCBAtom *atom_return
        )
{
    unsigned int i;
    XCBGenericError *err = NULL;
    xcb_intern_atom_reply_t *rep = NULL;
    for(i = 0; i < WMLast; ++i)
    {   
        rep = xcb_intern_atom_reply(display, (xcb_intern_atom_cookie_t) { cookies[i].sequence }, &err);
        if(rep)
        {   atom_return[i] = rep->atom;
        }
        else
        {   atom_return[i] = 0;
        }
        if(err)
        {   
            free(err);
            err = NULL;
        }
        free(rep);
    }
}

void 
XCBInitNetWMAtomsCookie(
        XCBDisplay *display, 
        XCBCookie *net_cookie_return
        )
{
    unsigned int i;
    for(i = 0; i < NetLast; ++i)
    {   net_cookie_return[i] = (XCBCookie) { .sequence = xcb_intern_atom(display, False, NET_WM_ATOM_LEN[i], NET_WM_ATOM_NAMES[i]).sequence };
    }
}

void 
XCBInitNetWMAtomsReply(
        XCBDisplay *display, 
        XCBCookie *cookies,
        XCBAtom *atom_return
        )
{
    unsigned int i;
    XCBGenericError *err = NULL;
    xcb_intern_atom_reply_t *rep = NULL;
    for(i = 0; i < NetLast; ++i)
    {   
        rep = xcb_intern_atom_reply(display, (xcb_intern_atom_cookie_t) { cookies[i].sequence }, &err);
        if(rep)
        {   atom_return[i] = rep->atom;
        }
        else
        {   atom_return[i] = 0;
        }
        if(err)
        {   
            free(err);
            err = NULL;
        }
        free(rep);
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


char *
XCBGetWindowName(
        XCBDisplay *display, 
        XCBWindow win
        )
{
    XCBCookie wmnamecookie = XCBGetWMNameCookie(display, win);
    XCBTextProperty textprop;
    uint8_t textstatus = 0;
    char *ret = NULL;

    textstatus = XCBGetWMNameReply(display, wmnamecookie, &textprop);

    if(textstatus)
    {   ret = textprop.name;
    }

    return ret;
}


XCBCookie
XCBGetPidCookie(
        XCBDisplay *display,
        XCBWindow win,
        XCBAtom _NET_WM_PID_ID
        )
{
    const uint8_t INITAL_BYTE_OFFSET = 0;
    const uint8_t LENGTH = 1;   /* Pid is just 1 uint32_t */
    XCBCookie ret = XCBGetWindowPropertyCookie(display, win, _NET_WM_PID_ID, INITAL_BYTE_OFFSET, LENGTH, False, XCB_ATOM_CARDINAL);

    return ret;
}

int32_t
XCBGetPidReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{    
    XCBWindowProperty *prop = XCBGetWindowPropertyReply(display, cookie);
    int32_t pid = -1;
    if(prop)
    {   
        void *data = XCBGetWindowPropertyValue(prop);
        uint32_t len = XCBGetPropertyValueLength(prop, sizeof(int8_t));
        if(data)
        {   
            const uint8_t SIZE_CONVERSION_MULT = 8;
            /* pid is always just 1 int32_t */
            if(len * SIZE_CONVERSION_MULT  == prop->format)
            {   
                switch(prop->format)
                {   
                    case 32:
                        pid = *(int32_t *)data;
                        break;
                    case 16:
                        pid = *(int16_t *)data;
                        break;
                    case 8:
                        pid = *(int8_t *)data;
                        break;
                    default:
                        break;
                }
            }
        }
        free(prop);
    }
    return pid;
}

