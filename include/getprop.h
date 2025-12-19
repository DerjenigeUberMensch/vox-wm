#ifndef _WM_GET_PROP_H
#define _WM_GET_PROP_H

#include "XCB-TRL/xcb_trl.h"
#include "prop.h"
#include "queue.h"

void PropListen(XCBDisplay *display, XCBWindow win, enum PropertyType type);
void PropListenArg(XCBDisplay *display, XCBWindow win, enum PropertyType type, PropArg arg);

#endif
