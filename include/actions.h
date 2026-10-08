#ifndef __WM__ACTIONS__H__
#define __WM__ACTIONS__H__

#include "main.h"

void ActionUserStats(void);

void ActionStickWindow(Client *c);

void ActionKillWindow(Client *c);

void ActionTerminateWindow(Client *c);

void ActionSetWindowLayout(Monitor *m, uint16_t layout);

void ActionSpawnWindow(char *file_to_run, const char *argv[]);

void ActionMaximizeWindow(Client *c);

void ActionMaximizeWindowVertical(Client *c);

void ActionMaximizeWindowHorizontal(Client *c);

void ActionRestart(void);

void ActionRestartQ(void);

void ActionQuit(void);

void ActionToggleStatusBar(Monitor *m);

void ActionToggleFullscreen(Client *c);

void ActionToggleDesktop(Monitor *m, uint16_t index);

#endif