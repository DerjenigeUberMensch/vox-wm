#ifndef __XCB__XINERAMA__H__
#define __XCB__XINERAMA__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xcb/xinerama.h>
#include "xcb_trl.h"

typedef xcb_xinerama_query_version_reply_t XCBXineramaQueryVersion;



XCBCookie
XCBXineramaQueryVersionCookie(
        XCBDisplay *display,
        uint8_t major_version,
        uint8_t minor_version
        );

/*
 * RETURN: XCBXineramaQueryVersion * on Success.
 * RETURN: NULL on Failure.
 */
XCBXineramaQueryVersion *
XCBXineramaQueryVersionReply(
        XCBDisplay *display,
        XCBCookie cookie
        );


#ifdef __cplusplus
}
#endif

#endif
