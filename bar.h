#ifndef BAR_H_
#define BAR_H_

#include "xcb_trl.h"


void writebar(char *fmt, ...);








/* Stuff we need
 * _NET_WM_WINDOW_TYPE:
 *      _NET_WM_WINDOW_TYPE_DOCK
 *      _NET_WM_WINDOW_TYPE_UTILITY
 *      // probably not
 *          _NET_WM_WINDOW_TYPE_TOOLBAR
 *          _NET_WM_WINDOW_TYPE_MENU
 * 
 * Maybe use the 0xFFFFFFFF suggestion? in _NET_WM_DESKTOP
 *
 * Prob could also compare XCB_ATOM_WM_NAME in the get property section?
 *
 *
 * needs to be set -> _NET_WM_STICKY
 *
*/
















#endif
