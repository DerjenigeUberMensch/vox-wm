#ifndef _WM_DECORATIONS_H_
#define _WM_DECORATIONS_H_


#include "../tools/XCB-TRL/xcb_trl.h"

typedef struct Decoration Decoration;


struct Decoration
{
    /* TODO */
    uint16_t w;
    uint16_t h;
    XCBWindow win;
    XCBWindow child;
    uint8_t flags;
    uint8_t pad0[3];
};

/* Allocates a decoration with all properties set to 0 or NULL. 
 * RETURN: Decoration * on Success.
 * RETURN: NULL on Failure.
 */
Decoration *X11DecorCreate(void);
/*
 * NOTE: Only 1 child can be held by a decoration.
 */
void X11DecorHoldChild(Decoration *decor, XCBWindow child);
/* Maps associated decoration window.
 */
void X11DecorMap(XCBDisplay *display, Decoration *decor);

void X11DecorPreferCSD(Decoration *decor);


#endif
