#ifndef __WM__INTERACTIVE__H__
#define __WM__INTERACTIVE__H__

#include "keybind/keybinds.h"
#include "main.h"

int DragWindow(XCBWindow window, XCBButton button);
int ResizeWindow(XCBWindow window, XCBButton button, bool alt_mode);

#endif