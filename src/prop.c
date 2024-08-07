




#include "XCB-TRL/xcb_winutil.h"
#include "prop.h"


extern XCBAtom netatom[];
extern XCBAtom wmatom[];
extern XCBAtom motifatom;

XCBCookie
PropGetTransientCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetTransientForHintCookie(display, win);
}

XCBCookie
PropGetWindowStateCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMState], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

XCBCookie
PropGetWindowTypeCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMWindowType], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ATOM);
}

XCBCookie
PropGetSizeHintsCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMNormalHintsCookie(display, win);
}

XCBCookie
PropGetWMHintsCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMHintsCookie(display, win);
}

XCBCookie
PropGetWMClassCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMClassCookie(display, win);
}

XCBCookie
PropGetWMProtocolCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetWMProtocolsCookie(display, win, wmatom[WMProtocols]);
}

XCBCookie
PropGetStrutCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_LENGTH = 4;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrut], NO_BYTE_OFFSET, STRUT_LENGTH, False, XCB_ATOM_CARDINAL);
}

XCBCookie
PropGetStrutpCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t STRUT_P_LENGTH = 12;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMStrutPartial], NO_BYTE_OFFSET, STRUT_P_LENGTH, False, XCB_ATOM_CARDINAL);
}

XCBCookie
PropGetNetWMNameCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMName], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, netatom[NetUtf8String]);
}

XCBCookie
PropGetWMNameCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, XCB_ATOM_WM_NAME, NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_STRING);
}

XCBCookie
PropGetPidCookie(XCBDisplay *display, XCBWindow win)
{   return XCBGetPidCookie(display, win, netatom[NetWMPid]);
}

XCBCookie
PropGetIconCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint32_t REQUEST_MAX_NEEDED_ITEMS = UINT32_MAX;
    return XCBGetWindowPropertyCookie(display, win, netatom[NetWMIcon], NO_BYTE_OFFSET, REQUEST_MAX_NEEDED_ITEMS, False, XCB_ATOM_ANY);
}

XCBCookie
PropGetMotifHintsCookie(XCBDisplay *display, XCBWindow win)
{   
    const uint8_t NO_BYTE_OFFSET = 0;
    const uint8_t MOTIF_WM_HINT_LENGTH = 5;
    return XCBGetWindowPropertyCookie(display, win, motifatom, NO_BYTE_OFFSET, MOTIF_WM_HINT_LENGTH, False, motifatom);
}
