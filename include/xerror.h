#ifndef __WM__X__ERROR__H__
#define __WM__X__ERROR__H__

#include "XCB-TRL/xcb_trl.h"
#include "util.h"


void INIT_XERROR(void);

void XERROR_ADD_SKIP_ID(XCBWindow id);

void DESTROY_XERROR(void);

/* Error handler */
void NonNullArg(1) xerror(XCBDisplay *display, XCBGenericError *error);


#endif