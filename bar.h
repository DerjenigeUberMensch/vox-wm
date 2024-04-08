#ifndef BAR_H_
#define BAR_H_

#include <stdint.h>

#include "xcb_trl.h"



#define _SHOW_BAR       ((1 << 1))

#define SHOWBAR(B)          (((B)->flags & _SHOW_BAR))


typedef struct Bar Bar;

struct Bar
{
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    uint8_t flags;

    XCBWindow win;
};


void setshowbar(Bar *bar, uint8_t state);

#endif
