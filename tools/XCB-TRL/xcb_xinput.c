#include "xcb_xinput.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/extensions/XI2.h>


/* count bits */
/* http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel */
const uint8_t
__bit_len_32(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = ((x + ((x >> 4) & 0xf0f0f0f)) * 0x1010101) >> 24;
    return x;
}


XCBCookie
XCBIQueryVersionCookie(
        XCBDisplay *display,
        uint16_t major,
        uint16_t minor
        )
{
    const xcb_input_xi_query_version_cookie_t cookie = xcb_input_xi_query_version(display, major, minor);
    const XCBCookie ret = { .sequence = cookie.sequence };

    return ret;
}

XCBIQueryVersion *
XCBIQueryVersionReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    const xcb_input_xi_query_version_cookie_t _cookie = { .sequence = cookie.sequence };
    xcb_input_xi_query_version_reply_t *rep = xcb_input_xi_query_version_reply(display, _cookie, &err);
    if(err)
    {   
        XCBSendErrorP(display, err);
        free(rep);
        return NULL;
    }
    return rep;
}

void
XCBIInitializeMask(
        XCBIEventMask *eventmask
        )
{
    memset(eventmask, 0, sizeof(XCBIEventMask));
}

void
XCBISetMask(
        XCBIEventMask *eventmasks,
        XCBXIEventMask mask
        )
{
    eventmasks->mask |= mask;
}

void
XCBIUnsetMask(
        XCBIEventMask *eventmasks,
        XCBXIEventMask mask
        )
{
    eventmasks->mask &= ~(mask);
}

void
XCBISetDevice(
        XCBIEventMask *eventmasks,
        XCBIDeviceId id
        )
{
    eventmasks->id = id;
}

XCBCookie
XCBISelectEvents(
        XCBDisplay *display,
        XCBWindow win,
        XCBIEventMask *eventmasks
        )
{
    const uint8_t MAX_EVENT_MASK_LENGTH = 26;
    uint8_t length = __bit_len_32(eventmasks->mask);

    if(length > MAX_EVENT_MASK_LENGTH)
    {   length = MAX_EVENT_MASK_LENGTH;
    }

    const size_t SIZE_REAL = sizeof(xcb_input_event_mask_t);
    const size_t SIZE = MAX_EVENT_MASK_LENGTH + SIZE_REAL;
    const xcb_input_xi_event_mask_t ev[SIZE];


    xcb_input_event_mask_t mask;
    mask.deviceid = eventmasks->id;
    mask.mask_len = 1;

    memcpy((void *)ev, &mask, SIZE_REAL);

    /*
    xcb_input_xi_event_mask_t *start = (xcb_input_xi_event_mask_t *)((uint8_t *)ev + SIZE_REAL);
    const xcb_input_xi_event_mask_t *end = (xcb_input_xi_event_mask_t *)(ev + length);
    uint32_t x = 0;
    uint32_t _mask;
    while(start != end)
    {
        _mask = eventmasks->mask & (1 << x);
        if(_mask)
        {   
            memcpy(start, &_mask, SIZE_REAL);
            ++start;
        }
        ++x;
    }
    */
    memcpy((uint8_t *)ev + SIZE_REAL, &eventmasks->mask, sizeof(uint32_t));
    XCBCookie ret = xcb_input_xi_select_events(display, win, 1, (void *)ev);
    return ret;
}


/* Events
 * XCB_INPUT_KEY_PRESS
 * XCB_INPUT_KEY_RELEASE
 * XCB_INPUT_BUTTON_PRESS 
 * XCB_INPUT_BUTTON_RELEASE
 * XCB_INPUT_MOTION
 * XCB_INPUT_ENTER 
 * XCB_INPUT_LEAVE 
 * XCB_INPUT_FOCUS_IN 
 * XCB_INPUT_FOCUS_OUT 
 * XCB_INPUT_HIERARCHY 
 * XCB_INPUT_PROPERTY
 * XCB_INPUT_RAW_KEY_PRESS
 * XCB_INPUT_RAW_KEY_RELEASE
 * XCB_INPUT_RAW_BUTTON_PRESS
 * XCB_INPUT_RAW_BUTTON_RELEASE
 * XCB_INPUT_RAW_MOTION
 * XCB_INPUT_TOUCH_BEGIN
 * XCB_INPUT_TOUCH_UPDATE 
 * XCB_INPUT_TOUCH_END
 * XCB_INPUT_TOUCH_OWNERSHIP
 * XCB_INPUT_RAW_TOUCH_BEGIN
 * XCB_INPUT_RAW_TOUCH_UPDATE
 * XCB_INPUT_RAW_TOUCH_END
 * XCB_INPUT_BARRIER_HIT
 * XCB_INPUT_BARRIER_LEAVE
 * XCB_INPUT_GESTURE_PINCH_BEGIN
 * XCB_INPUT_GESTURE_PINCH_UPDATE
 * XCB_INPUT_GESTURE_PINCH_END
 * XCB_INPUT_GESTURE_SWIPE_BEGIN 
 * XCB_INPUT_GESTURE_SWIPE_UPDATE
 * XCB_INPUT_GESTURE_SWIPE_END
 * XCB_INPUT_SEND_EXTENSION_EVENT
 *
 *
 * event structures: 
 * xcb_input_event_for_send_t
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */



