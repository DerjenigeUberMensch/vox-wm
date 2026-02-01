#ifndef __WM__WATCHERS__H__
#define __WM__WATCHERS__H__

#include "FNotify/fnotify.h"

#include "util.h"

int WatcherInit(void);
int WatcherAdd(char *FILE_NAME, void (*func)(Generic *arg), Generic *arg, enum FNotifyFlags flags);
void WatcherDestroy(void);


#endif
