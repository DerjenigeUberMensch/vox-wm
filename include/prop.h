#ifndef _PROP_HANDLING_H_
#define _PROP_HANDLING_H_


#include "XCB-TRL/xcb_trl.h"


XCBCookie GetTransientCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetWindowStateCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetWindowTypeCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetSizeHintsCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetWMHintsCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetWMClassCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetWMProtocolCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetStrutCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetStrutpCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetNetWMNameCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetWMNameCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetPidCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetIconCookie(XCBDisplay *display, XCBWindow window);
XCBCookie GetMotifHintsCookie(XCBDisplay *display, XCBWindow window);




#endif
