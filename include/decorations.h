#ifndef _WM_DECORATIONS_H_
#define _WM_DECORATIONS_H_

#include "../tools/XCB-TRL/xcb_trl.h"

#include "util.h"

typedef struct Decoration Decoration;

struct Client;

struct 
Decoration
{
    /* TODO */
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    XCBWindow win;
    uint8_t holding;
};

Decoration *createdecoration(void);
void NonNullAll decorationhold(Decoration *decor, struct Client *c);
void NonNullAll decorationrelease(Decoration *decor, struct Client *c);
void NonNullAll decorationupdate(Decoration *decor, struct Client *c);
void NonNullAll decorationfocus(Decoration *decor, struct Client *c, bool focus);
void NonNullAll cleanupdecoration(Decoration *decor);
uint32_t NonNull ISDECORACTIVE(struct Client *c);

#endif
