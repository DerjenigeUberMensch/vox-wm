#ifndef XCB_WMUTIL_H_
#define XCB_WMUTIL_H_

#include "xcb_trl.h"

/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerKeyPress(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerKeyRelease(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerButtonPress(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerButtonRelease(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerMotionNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerEnterNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerLeaveNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerFocusIn(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerFocusOut(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerKeymapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerExpose(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerGraphicsExpose(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerNoExpose(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerConfigureRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerCirculateRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerMapRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerResizeRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerCirculateNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerConfigureNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerCreateNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerDestroyNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerGravityNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerMapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerGravityNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerMapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerMappingNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerUnmapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerVisibilityNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerReparentNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerColormapNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerClientMessage(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerPropertyNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerSelectionRequest(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerSelectionClear(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerSelectionNotify(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerGenericEvent(
    XCBDisplay *display,
    XCBGenericEvent *event
    );
/* Default XCB Handling for events (assuming no window manager was running).
 */
void
XCBWMHandlerErrorHandler(
    XCBDisplay *display,
    XCBGenericEvent *event
    );



#endif
