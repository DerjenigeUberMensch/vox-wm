#ifndef _WM_GET_PROP_H
#define _WM_GET_PROP_H

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
    PropManage,
    PropUnmanage,

    /* Net State Setters */
    PropSetWtype,
    PropUnsetWtype,
    PropSetWState,
    PropUnsetWState,

    /* special */
    PropExitThread,
    PropLAST,
};

typedef union PropArg PropArg;

union PropArg
{
    void *v;
    void **vv;

    /* char */
    int8_t c[8];
    uint8_t uc[8];

    /* short */
    int16_t s[4];
    uint16_t us[4];

    /* int */
    int32_t i[2];
    uint32_t ui[2];

    /* long */
    int64_t l[1];
    uint64_t ul[1];
};

void PropInit(void);
void PropDestroy(void);
void PropListen(XCBDisplay *display, XCBWindow win, enum PropertyType type);
void PropListenArg(XCBDisplay *display, XCBWindow win, enum PropertyType type, PropArg arg);


#endif
