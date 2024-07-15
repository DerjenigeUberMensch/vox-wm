#include "xcb_image.h"

XCBImage *
XCBCreateImage(
        uint8_t depth,
        XCBImageFormat format,
        uint8_t offset,     /* xpad */
        uint8_t *data,         /* base */
        uint16_t width,
        uint16_t height,
        XCBImageOrder bitmap_pad,
        XCBImageOrder bytes_per_line
        )
{
    return NULL;
}

/*
 */
XCBCookie
XCBPutImage(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBImage *image,
        int32_t x,
        int32_t y,
        uint8_t left_pad
        )
{
    XCBCookie ret = xcb_image_put(display, drawable, gc, image, x, y, left_pad);

    return ret;
}
/*
 */
XCBCookie
XCBPutPixels(
        XCBDisplay *display,
        XCBGC gc,
        XCBDrawable drawable,
        uint8_t XCB_IMAGE_FORMAT_XXX,
        uint8_t depth,
        uint8_t left_pad,
        int32_t destination_x,
        int32_t destination_y,
        uint32_t width,
        uint32_t height,
        uint32_t data_len_size_in_bytes,
        const uint8_t *data
        )
{
    XCBCookie ret = xcb_put_image(display, XCB_IMAGE_FORMAT_XXX, drawable, gc, width, height, destination_x, destination_y, left_pad, depth, data_len_size_in_bytes, data);

    return ret;
}


XCBCookie
XCBFillRectangle(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        )
{
    const XCBRectangle rect = 
    {
        .x = x,
        .y = y,
        .width = w,
        .height = h
    };
    XCBCookie ret = xcb_poly_fill_rectangle(display, drawable, gc, 1, &rect);

    return ret;
}

XCBCookie
XCBFillRectangles(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBRectangle *rectangles,
        uint32_t nrectangles
        )
{
    XCBCookie ret = xcb_poly_fill_rectangle(display, drawable, gc, nrectangles, rectangles);

    return ret;
}

XCBCookie
XCBDrawRectangle(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        )
{
    const XCBRectangle rect = 
    {
        .x = x,
        .y = y,
        .width = w,
        .height = h
    };
    XCBCookie ret = xcb_poly_rectangle(display, drawable, gc, 1, &rect);

    return ret;
}

XCBCookie
XCBDrawRectangles(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBRectangle *rectangles,
        uint32_t nrectangles
        )
{
    XCBCookie ret = xcb_poly_rectangle(display, drawable, gc, nrectangles, rectangles);

    return ret;
}


XCBCookie
XCBFillPolygon(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBPoint *points,
        uint32_t npoints,
        uint8_t shape,
        uint8_t mode
        )
{
    XCBCookie ret = xcb_fill_poly(display, drawable, gc, shape, mode, npoints, points);

    return ret;
}

XCBCookie
XCBFillArc(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h,
        uint16_t angle1,
        uint16_t angle2
        )
{
    const XCBArc arc =
    {
        .x = x,
        .y = y,
        .width = w,
        .height = h,
        .angle1 = angle1,
        .angle2 = angle2
    };

    XCBCookie ret = xcb_poly_fill_arc(display,drawable, gc, 1, &arc);

    return ret;
}

XCBCookie
XCBFillArcs(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBArc *arcs,
        uint32_t number_of_arcs
        )
{
    XCBCookie ret = xcb_poly_fill_arc(display, drawable, gc, number_of_arcs, arcs);

    return ret;
}

XCBCookie
XCBDrawPoint(
        XCBDisplay *display, 
        uint8_t coordinatemode, 
        XCBDrawable drawable, 
        XCBGC gc, 
        int32_t x,
        int32_t y
        )
{
    const XCBPoint points = 
    {
        .x = x,
        .y = y
    };

    XCBCookie ret = xcb_poly_point(display, coordinatemode, drawable, gc, 1, &points);

    return ret;
}

XCBCookie
XCBDrawPoints(
        XCBDisplay *display, 
        uint8_t coordinatemode, 
        XCBDrawable drawable, 
        XCBGC gc, 
        uint32_t points_len, 
        XCBPoint *points
        )
{
    XCBCookie ret = xcb_poly_point(display, coordinatemode, drawable, gc, points_len, points);

    return ret;
}

XCBCookie
XCBDrawLine(
        XCBDisplay *display,
        uint8_t coordinatemode,
        XCBDrawable drawable,
        XCBGC gc,
        int32_t x1,
        int32_t y1,
        int32_t x2,
        int32_t y2
        )
{
    const XCBPoint points[2] = 
    { 
        /* point 1 */
        { 
            .x = x1, 
            .y = y1 
        }, 
        /* point 2 */
        { 
            .x = x2, 
            .y = y2
        }
    };

    XCBCookie ret = xcb_poly_line(display, coordinatemode, drawable, gc, 2, points);

    return ret;
}

XCBCookie
XCBDrawLines(
        XCBDisplay *display, 
        uint8_t coordinatemode, 
        XCBDrawable drawable, 
        XCBGC gc, 
        uint32_t points_len, 
        XCBPoint *points
        )
{
    XCBCookie ret = xcb_poly_line(display, coordinatemode, drawable, gc, points_len, points);

    return ret;
}














