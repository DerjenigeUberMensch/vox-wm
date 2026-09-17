

#include "xcb_xinerama.h"
#include "__private__xcb__err__.h"




XCBCookie
XCBXineramaQueryVersionCookie(
        XCBDisplay *display,
        uint8_t major_version,
        uint8_t minor_version
        )
{
    xcb_xinerama_query_version_cookie_t cookie = xcb_xinerama_query_version(display, major_version, minor_version);

    XCBCookie ret = { .sequence = cookie.sequence };

    _xcb_push_func(ret);

    return ret;
}

XCBXineramaQueryVersion *
XCBXineramaQueryVersionReply(
        XCBDisplay *display,
        XCBCookie cookie
        )
{
    XCBGenericError *err = NULL;
    xcb_xinerama_query_version_cookie_t cookie1 = { .sequence = cookie.sequence };
    XCBXineramaQueryVersion *reply = xcb_xinerama_query_version_reply(display, cookie1, &err);

    free(reply);
    free(err);

    return NULL;

}




