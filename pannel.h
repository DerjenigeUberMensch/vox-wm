#ifndef PANNEL_H_
#define PANNEL_H_

#include "XCB-TRL/xcb_trl.h" 
#include "util.h"
#include <stdint.h>
#include <xcb/xcb_image.h>

typedef struct Pannel Pannel;
typedef struct PannelWidget PannelWidget;

struct Pannel
{
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    uint8_t flags;
    uint8_t pad[3];

    XCBWindow win;
    XCBPixmap pix;
    XCBGC gc;
    XCBDisplay *dpy;
    unsigned int screen;

    size_t itemsize;
    uint32_t bufflength;
    char *buff;
};

struct PannelWidget
{
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    size_t itemsize;
    uint32_t bufflength;
    char *buff;
};


/*
 * NOTE: No side effects if resize fails.
 *
 * RETURN: 0 On Success.
 * RETURN: 1 On Failure.
 */
int
PannelResizeBuff(
        Pannel *pannel,
        uint32_t w,
        uint32_t h
        );
/* Builtin pannel Stuff */

Pannel *
PannelCreate(
    XCBDisplay *display,
    int32_t screen,
    XCBWindow parent, 
    int32_t x, 
    int32_t y, 
    int32_t w, 
    int32_t h
    );

void
PannelDestroy(
    Pannel *pannel
        );

PannelWidget *
PannelWidgetCreate(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        uint16_t w,
        uint16_t h
        );
/* Resizes a widget based on parametors. Widget is assumed to be empty, as data WILL be destroyed.
 * 
 * RETURN: 1 on Failure.
 * RETURN: 0 on Success.
 */
int
PannelWidgetResize(
        Pannel *pannel,
        PannelWidget *widget,
        uint16_t w,
        uint16_t h
        );

/* Copies data to widget buffer using specified coordinates,
 * char *buffcopy is simply char * to get constant size, 
 * real data is interpreted by user.
 *
 * RETURN: 1 on Failure.
 * RETURN: 0 on Success.
 */
int
PannelWidgetWrite(
        Pannel *pannel,
        PannelWidget *widget,
        char *buffcopy,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        );
/* TOD */
XCBCookie
PannelDrawBuff(
    Pannel *pannel,
    int16_t xoffset,
    int16_t yoffset,
    const uint16_t w,
    const uint16_t h
        );

/*
 * id1 id2 id3 * 256
 * I think the only real solution is to have the client print from back to frot.
 * EX:
 * last lastfocus laststack
 * And just print down from there disregardning the client, but the client just has info of the client .
 * Some issues may arrise if the window is 0, this also is implementation reliant.
 * TODO
 */


#endif
