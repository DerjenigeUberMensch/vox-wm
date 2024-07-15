#ifndef _XCB_PTL_XINPUT_H_
#define _XCB_PTL_XINPUT_H_

#include "xcb_trl.h"

#include <xcb/xinput.h>

typedef xcb_input_xi_query_version_reply_t XCBIQueryVersion;
typedef xcb_input_xi_event_mask_t XCBXIEventMask;
typedef xcb_input_device_id_t XCBIDeviceId;

typedef struct XCBIEventMask XCBIEventMask;

struct XCBIEventMask
{
    XCBIDeviceId id;
    uint32_t mask;
};



XCBCookie
XCBIQueryVersionCookie(
        XCBDisplay *display,
        uint16_t major,
        uint16_t minor
        );

XCBIQueryVersion *
XCBIQueryVersionReply(
        XCBDisplay *display,
        XCBCookie cookie
        );


void
XCBIInitializeMask(
        XCBIEventMask *eventmask
        );

void
XCBISetMask(
        XCBIEventMask *eventmasks,
        XCBXIEventMask mask
        );

void
XCBIUnsetMask(
        XCBIEventMask *eventmasks,
        XCBXIEventMask mask
        );

void
XCBISetDevice(
        XCBIEventMask *eventmasks,
        XCBIDeviceId id
        );


XCBCookie
XCBISelectEvents(
        XCBDisplay *display,
        XCBWindow win,
        XCBIEventMask *eventmasks
        );












#endif
