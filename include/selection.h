#ifndef __WM__ICCCM__SELECTION_H
#define __WM__ICCCM__SELECTION_H

#include "main.h"


/* Check if another window manager is running on the same screen. */
void checkotherwmsn(void);

void setupselection(void);
int setupwmsn(XCBEvent *ev, Arg arg);

#endif
