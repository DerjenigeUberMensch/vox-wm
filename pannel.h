#ifndef PANNEL_H_
#define PANNEL_H_


#include <stdint.h>
#include <xcb/xcb_image.h>

#include "XCB-TRL/xcb_trl.h" 
#include "util.h"
#include "fonts.h"

typedef struct Pannel Pannel;
typedef struct PannelWidget PannelWidget;

struct Pannel
{
    uint16_t w;
    uint16_t h;

    uint8_t flags;
    uint8_t running;
    uint8_t pad[2];

    XCBWindow win;
    XCBGC gc;
    XCBPixmap pix;
    uint16_t pixw;
    uint16_t pixh;
    XCBDisplay *dpy;
    int screen;

    uint32_t widgetslen;
    PannelWidget *widgets;
    pthread_mutex_t *mut;
    pthread_spinlock_t *qmut;
    pthread_cond_t *cond;
    XFFont *font;
    void (*draw)(Pannel *);
};

struct PannelWidget
{
    int16_t x;
    int16_t y;

    uint16_t w;
    uint16_t h;

    XCBPixmap pix;
    uint16_t pixw;
    uint16_t pixh;
};


/*
 * Data is destroyed on resize.
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


void
PannelRedraw(
        Pannel *p,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        );
/* Builtin pannel Stuff */

Pannel *
PannelCreate(
    XCBWindow parent, 
    int32_t x, 
    int32_t y, 
    int32_t w, 
    int32_t h
    );

void
PannelClear(
        Pannel *pannel
        );

/* Documentation Purposes.
 * Automatically destroyed on Unmap/destroy notify.
 * this function should not be used, aside from testing.
 */
void
PannelDestroy(
    Pannel *pannel
        );

void
PannelDrawText(
        Pannel *pannel, 
        int32_t x, 
        int32_t y, 
        uint32_t len,
        char *str
        );


/* Data should not be freed instead to free all data use PannelDestroy();
 * Do note that this pointer should be recycled and not thrown away as that would poison the pool.
 *
 * RETURN: PannelWidget * On Success.
 * RETURN: NULL On Failure.
 */
PannelWidget *
PannelWidgetCreate(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        uint16_t w,
        uint16_t h
        );
/* Data will be destroyed
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

void
PannelWritePixel(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        uint32_t colour
        );

void
PannelDrawLine(
        Pannel *pannel,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        uint32_t colour
        );

void
PannelDrawRectangle(
        Pannel *pannel,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        uint8_t fill,
        uint32_t colour
        );

/* Overflow results in undefined behaviour.
 */
void
PannelWidgetDrawLine(
        Pannel *pannel,
        PannelWidget *widget,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        uint32_t colour
        );

void
PannelWidgetDrawRectangle(
        Pannel *pannel,
        PannelWidget *wi,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        uint8_t fill,
        uint32_t colour
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
