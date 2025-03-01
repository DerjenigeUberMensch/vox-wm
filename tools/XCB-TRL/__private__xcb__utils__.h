#ifndef __XCB__PRIVATE__UTILS__H__
#define __XCB__PRIVATE__UTILS__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>

#include "safebool.h"
#include "__private__xcb__err__.h"

#ifdef XCB_TRL_ENABLE_DEBUG

#define _XCB_MANUAL_DEBUG(fmt, ...)             (fprintf(stderr, "_XCB_DEBUG_ [%s:%d] by %s(): " fmt "\n", __FILE__,__LINE__,__func__,__VA_ARGS__))
#define _XCB_MANUAL_DEBUG0(X)                   (fprintf(stderr, "_XCB_DEBUG_ [%s:%d] by %s(): " X "\n", __FILE__, __LINE__, __func__))

#else

#define _XCB_MANUAL_DEBUG(fmt, ...)             ((void)0)
#define _XCB_MANUAL_DEBUG0(X)                   ((void)0)

#endif

/* gcc */
#if defined(__GNUC__) || defined(__clang__)

#ifndef __RETURN__NEVER__NULL__
#define __RETURN__NEVER__NULL__ __attribute__((returns_nonnull))
#endif

#ifndef __DEPRECATED__
#define __DEPRECATED__ __attribute__((deprecated))
#endif

/* Indicates that a function never returns (includes void).
 * What does this mean?;
 * A.) This function always results in a infinite loop;
 * b.) This function calls a variation of exit();
 */
#ifndef __FUNC__EXIT__POINT__
#define __FUNC__EXIT__POINT__ __attribute__((noreturn))
#endif

#ifndef __cplusplus
#define fallthrough __attribute__((fallthrough))
#endif

#ifndef UNREACHABLE
#define UNREACHABLE __builtin_unreachable();
#endif

#ifndef expect
#define expect(VARIABLE, EXPECTED_VALUE)    __builtin_expect(VARIABLE, EXPECTED_VALUE)
#endif

#ifndef likely
#define likely(X)    __builtin_expect(!!(X), 1)
#endif

#ifndef unlikely
#define unlikely(X) __builtin_expect(!!(X), 0)
#endif
#else

#ifndef __RETURN__NEVER__NULL__
#define __RETURN__NEVER__NULL__ 
#endif

#ifndef __DEPRECATED__
#define __DEPRECATED__ 
#endif

#ifndef __FUNC__EXIT__POINT__
#define __FUNC__EXIT__POINT__ 
#endif

#ifndef __cplusplus
#define fallthrough 
#endif

#ifndef UNREACHABLE
#define UNREACHABLE
#endif

#ifndef expect
#define expect(VARIABLE, EXPECTED_VALUE)    (VARIABLE == EXPECTED_VALUE)
#endif


#ifndef likely
#define likely(X)       (X)
#endif

#ifndef unlikely
#define unlikely(X)     (X)
#endif
#endif



xcb_screen_t *
screen_of_display(
        xcb_connection_t *display, 
        int screen
        );

#ifdef __cplusplus
}
#endif


#endif
