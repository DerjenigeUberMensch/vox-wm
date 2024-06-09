#ifndef XFF_FONTS_H_
#define XFF_FONTS_H_


#include "XCB-TRL/xcb_trl.h"







typedef struct XFFont XFFont;




struct XFFont
{
    XCBFont font;
    XCBGC gc;
    
    int32_t descent;
    uint32_t height;
    uint32_t width;
    uint16_t char_max;
    uint16_t char_min;
    xcb_charinfo_t *width_lut;
};


XFFont *
XFFontCreate(
        XCBDisplay *display,
        XCBPixmap pix,
        char *fontname
        );

int
XFFontHasGlyph(
        XFFont *font, 
        const uint16_t character
        );


void
XFFontDrawText(
        XCBDisplay *display,
        XFFont *font,
        XCBDrawable drawable,
        int32_t x,
        int32_t y,
        uint32_t len,
        uint16_t *text
        );


void
XFFontDestroy(
        XCBDisplay *display,
        XFFont *font
        );











#endif
