#ifndef _WM_PROPERTY_NOTIFY_H_
#define _WM_PROPERTY_NOTIFY_H_

#include "XCB-TRL/xcb_trl.h"

enum PropertyType
{
    PropNone,
    PropTransient,
    PropWindowState,
    PropWindowType,
    PropSizeHints,
    PropWMHints,
    PropWMClass,
    PropWMProtocol,
    PropStrut,
    PropStrutp,

    PropNetWMName,
    PropWMName,
    PropPid,
    PropIcon,
    PropMotifHints,

    /* special */
    PropExitThread,
    PropLAST,
};

void PropInit();
void PropDestroy(void);
void PropListen(XCBDisplay *display, XCBWindow win, enum PropertyType type);


#endif
