#ifndef _PROP_HANDLING_H_
#define _PROP_HANDLING_H_


#include "XCB-TRL/xcb_trl.h"


XCBCookie PropGetTransientCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetWindowStateCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetWindowTypeCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetSizeHintsCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetWMHintsCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetWMClassCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetWMProtocolCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetStrutCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetStrutpCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetNetWMNameCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetWMNameCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetPidCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetIconCookie(XCBDisplay *display, XCBWindow window);
XCBCookie PropGetMotifHintsCookie(XCBDisplay *display, XCBWindow window);




#endif
