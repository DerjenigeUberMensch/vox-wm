#ifndef _WM_X_H
#define _WM_X_H

/* X11 Helper functions */



#include <stdint.h>
#include "../tools/XCB-TRL/xcb_trl.h"
#include "../tools/XCB-TRL/xcb_winutil.h"
#include "../tools/XCB-TRL/xcb_gtk.h"
#include "../tools/XCB-TRL/xcb_xembed.h"




/* Checks if a given number would be sticky in the wm-spec.
 * RETURN: NonZero on True.
 * RETURN: 0 on False.
 */
uint8_t checksticky(int64_t x);
/* Allocates memory and resturns the pointer in **str_return from the specified XCBWindowProperty. */
char *getnamefromreply(XCBWindowProperty *namerep);
/* Gets the icon property from the specified XCBWindowProperty. */
uint32_t *geticonprop(XCBWindowProperty *iconreply);


/* Getters */
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

char *GetAtomNameQuick(XCBDisplay *display, XCBAtom atom);

#endif
