#ifndef PANNEL_H_
#define PANNEL_H_

#include "XCB-TRL/xcb_trl.h"
#include "util.h"
#include <stdint.h>

typedef struct Pannel Pannel;
typedef struct PannelWidget PannelWidget;

struct Pannel
{
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    /* pix */
    XCBWindow win;
    PannelWidget **wigets;
};

struct PannelWidget
{
};


Pannel *
PannelCreate(
        i16 x,
        i16 y,
        u16 w,
        u16 h
        );
void
PannelMap(
        Pannel *pannel
        );
void
PannelUnmap(
        Pannel * pannel
        );


#endif
