#ifndef XCB_PTL_IMAGE_UTIL_H_
#define XCB_PTL_IMAGE_UTIL_H_


/* AKA XCBArgb
 *
 * BLUE     (0, 255) (unsigned char)
 * RED      (0, 255) (unsigned char)
 * GREEN    (0, 255) (unsigned char)
 * // Compositor required.
 * ALPHA    (0, 255) (unsigned char)
 *
 * FORMAT: BLUE + (GREEN << 8) + (RED << 16) + (ALPHA << 24).
 */
#define XCB_IMAGE_FORMAT_BGRA        (XCB_IMAGE_FORMAT_Z_PIXMAP)

#endif
