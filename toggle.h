#ifndef TOGGLE_H_
#define TOGGLE_H_
#include "dwm.h"

/* Mostly a testing function */
void UserStats(const Arg *arg);
/* Switch to a monitor based on the argument int arg i */
void FocusMonitor(const Arg *arg);
/* Changes the max number of master windows possible */
void ChangeMasterWindow(const Arg *arg);
/* Kills the current window */
void KillWindow(const Arg *arg);
/* Attempts to kill the current window directly instead of just sending a signal and waiting for the window to respond */
void TerminateWindow(const Arg *arg);
/* keybind to move the current window where the mouse cursor is */
void DragWindow(XCBDisplay *display, XCBWindow win, const XCBKeyCode key_or_button);
/* restarts dwm */
void Restart(const Arg *arg);
/* quits dwm */
void Quit(const Arg *arg);
/* resizes the current window based on mouse position */
void ResizeWindow(const Arg *arg);
/* sets the window layout based on a enum in dwm.h -> Grid, Floating, Monocle, Tiled */
void SetWindowLayout(const Arg *arg);
/* Sets the size of the master window 0.0 -> 1.0 for the Tiled Layout
 * where 1 is just monocle with extra steps */
void SetMonitorFact(const Arg *arg);
/* Spawns a window based on arguments provided */
void SpawnWindow(const Arg *arg);
/* Maximizes the currently selected window */
void MaximizeWindow(const Arg *arg);
/* Maximizes a window vertically */ 
void MaximizeWindowVertical(const Arg *arg);
/* Maximizes a window horizontally */
void MaximizeWindowHorizontal(const Arg *arg);
/* Switches to the next visible window based on user input */
void AltTab(const Arg *arg);
/* Tags a window AKA makes it visible in another desktop thing or "tag" */
void TagWindow(const Arg *arg);
/* Tags a monitor */
void TagMonitor(const Arg *arg);
/* Toggles if we show the Status bar or not */
void ToggleStatusBar(const Arg *arg);
/* This toggles the floating layout */
void ToggleFloating(const Arg *arg);
/* Toggles fullscreen mode for all windows in current tag */
void ToggleFullscreen(const Arg *arg);
/* Probably toggles the tag selected IDK */
void ToggleTag(const Arg *arg);
/* This is used in keybinds default SUPER + (1-9)
 * And essential just changes the tag to the one specified in the number
 * AKA 1 << TAG
 * This is really only a "User function" see View for devs
 */
void ToggleView(const Arg *arg);
/* Switches to the tag number specified in base power of 2^x
 * Example:
 * tag1,tag2,tag3,tag4,tag5,tag6,tag7,tag8,tag9
 * {1,  2,   4,   8,   16,  32,  64,  128, 256}
 */
void View(const Arg *arg);
/* Idk what this does */
void Zoom(const Arg *arg);

#endif
