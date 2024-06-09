


/* Mostly taken from lemonbar */
/*
 * Copyright (C) 2012 The Lemon Man
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */



#include "fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Apparently xcb cannot seem to compose the right request for this call, hence we have to do it by
// ourselves.
// The funcion is taken from 'wmdia' (http://wmdia.sourceforge.net/)
xcb_void_cookie_t xcb_poly_text_16_simple(xcb_connection_t * c,
        xcb_drawable_t drawable, xcb_gcontext_t gc, int16_t x, int16_t y,
        uint32_t len, const uint16_t *str)
{
    static const xcb_protocol_request_t xcb_req = {
        5,                // count
        0,                // ext
        XCB_POLY_TEXT_16, // opcode
        1                 // isvoid
    };
    struct iovec xcb_parts[7];
    uint8_t xcb_lendelta[2];
    xcb_void_cookie_t xcb_ret;
    xcb_poly_text_8_request_t xcb_out;

    xcb_out.pad0 = 0;
    xcb_out.drawable = drawable;
    xcb_out.gc = gc;
    xcb_out.x = x;
    xcb_out.y = y;

    xcb_lendelta[0] = len;
    xcb_lendelta[1] = 0;

    xcb_parts[2].iov_base = (char *)&xcb_out;
    xcb_parts[2].iov_len = sizeof(xcb_out);
    xcb_parts[3].iov_base = 0;
    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;

    xcb_parts[4].iov_base = xcb_lendelta;
    xcb_parts[4].iov_len = sizeof(xcb_lendelta);
    xcb_parts[5].iov_base = (char *)str;
    xcb_parts[5].iov_len = len * sizeof(int16_t);

    xcb_parts[6].iov_base = 0;
    xcb_parts[6].iov_len = -(xcb_parts[4].iov_len + xcb_parts[5].iov_len) & 3;

    xcb_ret.sequence = xcb_send_request(c, 0, xcb_parts + 2, &xcb_req);

    return xcb_ret;
}



XFFont *
XFFontCreate(
        XCBDisplay *display,
        XCBPixmap pix,
        char *fontname
        )
{
    xcb_query_font_cookie_t queryreq;
    xcb_query_font_reply_t *font_info;
    xcb_void_cookie_t cookie;
    xcb_font_t font;

    font = xcb_generate_id(display);

    cookie = xcb_open_font_checked(display, font, strlen(fontname), fontname);
    if (xcb_request_check (display, cookie)) 
    {
        fprintf(stderr, "Could not load font \"%s\"\n", fontname);
        return NULL;
    }

    XFFont *ret = malloc(sizeof(XFFont));

    queryreq = xcb_query_font(display, font);
    font_info = xcb_query_font_reply(display, queryreq, NULL);

    ret->font = font;
    ret->descent = font_info->font_descent;
    ret->height = font_info->font_ascent + font_info->font_descent;
    ret->width = font_info->max_bounds.character_width;
    ret->char_max = font_info->max_byte1 << 8 | font_info->max_char_or_byte2;
    ret->char_min = font_info->min_byte1 << 8 | font_info->min_char_or_byte2;
    ret->gc = xcb_generate_id(display);

    xcb_create_gc(display, ret->gc, pix, XCB_GC_FOREGROUND, (const uint32_t []) { ~0 });

    // Copy over the width lut as it's part of font_info
    size_t lut_size = sizeof(xcb_charinfo_t) * xcb_query_font_char_infos_length(font_info);
    if (lut_size) 
    {
        ret->width_lut = malloc(lut_size);
        memcpy(ret->width_lut, xcb_query_font_char_infos(font_info), lut_size);
    }

    free(font_info);
    return ret;

}

int
XFFontHasGlyph(
        XFFont *font, 
        const uint16_t character
        )
{
    if(font->char_min > character || font->char_max < character)
    {   return 0;
    }
    
    if(font->width_lut && font->width_lut[character - font->char_min].character_width == 0)
    {   return 0;
    }

    return 1;
}

void
XFFontDrawText(
        XCBDisplay *display,
        XFFont *font,
        XCBDrawable drawable,
        int32_t x,
        int32_t y,
        uint32_t len,
        uint16_t *text
        )
{
    int32_t i;
    uint8_t *utf = NULL;
    uint16_t ucs = 0;
    uint32_t pos_x = 0;
    const uint16_t UNKNOWN_CHAR_UTF8 = 0xFFFD;
    /* parse */
    for(i = 0 ; i < len; ++i)
    {
        utf = (uint8_t *)text + i;

        // ASCII
        if(utf[0] < 0x80)
        {   ucs = utf[0];
            ++i;
        }
        // Two byte utf8 sequence
        else if ((utf[0] & 0xe0) == 0xc0) {
            ucs = (utf[0] & 0x1f) << 6 | (utf[1] & 0x3f);
            i += 2;
        }
        // Three byte utf8 sequence
        else if ((utf[0] & 0xf0) == 0xe0) {
            ucs = (utf[0] & 0xf) << 12 | (utf[1] & 0x3f) << 6 | (utf[2] & 0x3f);
            i += 3;
        }
        // Four byte utf8 sequence
        else if ((utf[0] & 0xf8) == 0xf0) {
            ucs = 0xfffd;
            i += 4;
        }
        // Five byte utf8 sequence
        else if ((utf[0] & 0xfc) == 0xf8) {
            ucs = 0xfffd;
            i += 5;
        }
        // Six byte utf8 sequence
        else if ((utf[0] & 0xfe) == 0xfc) {
            ucs = 0xfffd;
            i += 6;
        }
        // Not a valid utf-8 sequence (AKA just char *)
        else 
        {
            ucs = utf[0];
            i  += 1;
        }

        if(!XFFontHasGlyph(font, ucs))
        {   ucs = UNKNOWN_CHAR_UTF8;
        }

        /* draw the character */
        uint32_t ch_width = (font->width_lut) ? font->width_lut[ucs - font->char_min].character_width : font->width;
        
        // xcb accepts string in UCS-2 BE, so swap
        ucs = (ucs >> 8) | (ucs << 8);

        xcb_poly_text_16_simple(display, drawable, font->gc, pos_x + x, y + font->height, 1, &ucs);
        pos_x += ch_width;
    }
}


void
XFFontDestroy(
        XCBDisplay *display,
        XFFont *font
        )
{
    if(font)
    {
        xcb_close_font(display, font->font);
        xcb_free_gc(display, font->gc);
        free(font->width_lut);
        free(font);
    }
}
