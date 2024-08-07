#ifndef _PROP_HANDLING_H_
#define _PROP_HANDLING_H_


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
typedef struct GetPropCookie GetPropCookie;

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

struct  
GetPropCookie
{
    XCBWindow win;
    enum PropertyType type;
    PropArg arg;
    XCBCookie cookie;
    uint8_t pad0[4];
};



XCBCookie
PropGetInvalidCookie(
        XCBDisplay *display, 
        XCBWindow win
        );

XCBCookie 
PropGetTransientCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetWindowStateCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetWindowTypeCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetSizeHintsCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetWMHintsCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetWMClassCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetWMProtocolCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetStrutCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetStrutpCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetNetWMNameCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetWMNameCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetPidCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetIconCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

XCBCookie 
PropGetMotifHintsCookie(
        XCBDisplay *display, 
        XCBWindow window
        );

void 
PropUpdateInvalid(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateTrans(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateWindowState(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateWindowType(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateSizeHints(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateWMHints(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateWMClass(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateWMProtocol(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateStrut(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateStrutP(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateNetWMName(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateWMName(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdatePid(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateIcon(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateMotifHints(
       XCBDisplay *display, 
       GetPropCookie *cookie
       );

void 
PropUpdateManage(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateUnmanage(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateSetWType(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateUnsetWType(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateSetWState(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );

void 
PropUpdateUnsetWState(
        XCBDisplay *display, 
        GetPropCookie *cookie
        );


void
PropUpdateProperty(
        XCBDisplay *display,
        GetPropCookie *cookie
        );

#endif
