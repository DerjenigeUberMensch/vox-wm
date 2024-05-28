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
    uint8_t running;
    uint8_t pad[2];

    XCBWindow win;
    XCBGC gc;
    XCBDisplay *dpy;
    int screen;

    size_t itemsize;
    uint64_t bufflength;
    char *buff;
    PannelWidget *widgets;
    uint32_t widgetslen;
    pthread_mutex_t *mut;
    pthread_spinlock_t *qmut;
    pthread_cond_t *cond;
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


void
PannelRedraw(
        Pannel *p
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
PannelDestroy(
    Pannel *pannel
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

void
PannelWidgetWrite(
        Pannel *pannel,
        PannelWidget *widget,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        void *buffcopy
        );
int
PannelWritePixel(
        Pannel *pannel,
        int16_t x,
        int16_t y,
        void *color
        );

int
PannelWriteBuff(
        Pannel *pannel,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        void *data
        );

/* Overflow results in undefined behaviour.
 */
void
PannelDrawLine(
        Pannel *pannel,
        int32_t startx,
        int32_t starty,
        int32_t endx,
        int32_t endy,
        void *color
        );

void
PannelDrawRectangle(
        Pannel *pannel,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        uint8_t fill,
        void *color
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
        void *color
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
        void *color
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
