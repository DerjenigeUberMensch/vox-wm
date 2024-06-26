#ifndef _WM_X_H
#define _WM_X_H

/* X11 Helper functions */



#include <stdint.h>
#include "XCB-TRL/xcb_trl.h"




/* Checks if a given number would be sticky in the wm-spec.
 * RETURN: NonZero on True.
 * RETURN: 0 on False.
 */
uint8_t checksticky(int64_t x);
/* Allocates memory and resturns the pointer in **str_return from the specified XCBWindowProperty. */
char *getnamefromreply(XCBWindowProperty *namerep);
/* Gets the icon property from the specified XCBWindowProperty. */
uint32_t *geticonprop(XCBWindowProperty *iconreply);


#endif
