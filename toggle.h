#ifndef TOGGLE_H_
#define TOGGLE_H_

#include "dwm.h"


/* Mostly a testing function */
void UserStats(const Arg *arg);
/* Switch to a monitor based on the argument int arg i */
void FocusMonitor(const Arg *arg);
/* Kills the current window */
void KillWindow(const Arg *arg);
/* Attempts to kill the current window directly instead of just sending a signal and waiting for the window to respond */
void TerminateWindow(const Arg *arg);
/* keybind to move the current window where the mouse cursor is */
void DragWindow(const Arg *arg);
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
/* Toggles if we show the Status bar or not */
void ToggleStatusBar(const Arg *arg);
/* Toggles fullscreen mode for all windows in current tag */
void ToggleFullscreen(const Arg *arg);
#endif
