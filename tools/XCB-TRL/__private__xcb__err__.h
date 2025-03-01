#ifndef __XCB__PRIVATE__ERROR__H__
#define __XCB__PRIVATE__ERROR__H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "xcb_trl_types.h"
#include "safebool.h"

#ifdef XCB_TRL_ENABLE_DEBUG
    #define _xcb_push_func(cookie) XCBDebugPushID(__func__, cookie.sequence)
#else
    #define _xcb_push_func(cookie) ((void)cookie)
#endif



/*
 * RETURN: true on valid format.
 * RETURN: false on invalid format.
 */
bool 
__XValidFormat(
        uint8_t x
        );
/* X11, GetProp.c
 * Protect against both integer overflow and just plain oversized
 * memory allocation - no server should ever return this many props.
 *
 * RETURN: true on valid size.
 * RETURN: false on invalid size.
 */
bool 
__XValidSize(
        uint32_t value_len
        );

/*
 * RETURN: true if reply is valid.
 * RETURN: false if reply is not valid.
 */
bool
__XValidReply(
        xcb_get_property_reply_t  *reply
        );

void *
__XValidateReply(
        xcb_connection_t *display,
        xcb_get_property_reply_t *reply,
        xcb_generic_error_t *error
        );

void *
__XCheckReply(
        xcb_connection_t *display,
        xcb_get_property_reply_t *reply,
        xcb_generic_error_t *error
        );

/*
 * Manually throw a XCBError not recommended to use, even for implementors.
 */
void
__XCBThrowError(
        xcb_connection_t *display,
        xcb_void_cookie_t cookie, 
        uint8_t error, 
        uint8_t major_code, 
        uint16_t minor_code
        );

void
__XCBSetErrorHandler(
        void (*error_handler)(
            XCBDisplay *, 
            XCBGenericError *
            )
        );

void 
XCBBreakPoint(
        void
        );

void 
XCBDebugPushID(
    const char *const function_name, 
    uint32_t sequence
    );





#ifdef __cplusplus
}
#endif




#endif
