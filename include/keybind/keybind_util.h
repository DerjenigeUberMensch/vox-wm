#ifndef __KEYBIND_UTIL_H__
#define __KEYBIND_UTIL_H__

#include "keybinds.h"

/* 
 * RETURN: EXIT_SUCCESS on Success.
 * RETURN: EXIT_FALURE on Failure.
 */
int KEYBIND_FIND_KEY(WMKeyBind*find, garray_i *index_return);

XCBKeyCode **WM_KEYBIND_GENERATE_KEYCODES_X11(GArray *syms);

void WM_KEYBIND_GRAB_KEYCODE_X11(XCBKeyCode code, u16 mod);

#endif