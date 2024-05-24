#ifndef XCB_PTL_IMAGE_TYPEDEF_H_
#define XCB_PTL_IMAGE_TYPEDEF_H_


#include <xcb/xcb_image.h>
#include "xcb_trl.h"



typedef struct xcb_image_t XCBImage;
typedef struct xcb_shm_segment_info_t XCBShmSegmentInfo;
typedef enum xcb_image_format_t XCBImageFormat;
typedef enum xcb_image_order_t XCBImageOrder;
typedef xcb_rectangle_t XCBRectangle;
typedef xcb_point_t XCBPoint;
typedef xcb_arc_t XCBArc;



/*TODO RETURNS NOTHING.
 */
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
        );
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
        );
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
        );

/*
 */
XCBCookie
XCBFillRectangle(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        int32_t x,
        int32_t y,
        uint32_t w,
        uint32_t h
        );

/*
 */
XCBCookie
XCBFillRectangles(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBRectangle *rectangles,
        uint32_t nrectangles
        );

/*
 */
XCBCookie
XCBFillPolygon(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBPoint *points,
        uint32_t npoints,
        uint8_t shape,
        uint8_t mode
        );

/*
 */
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
        );
/*
 */
XCBCookie
XCBFillArcs(
        XCBDisplay *display,
        XCBDrawable drawable,
        XCBGC gc,
        XCBArc *arcs,
        uint32_t number_of_arcs
        );
/*
 */
XCBCookie
XCBDrawPoint(
        XCBDisplay *display,
        uint8_t coordinate_mode,
        XCBDrawable drawable,
        XCBGC gc,
        uint32_t points_len,
        XCBPoint *points
        );









#endif
