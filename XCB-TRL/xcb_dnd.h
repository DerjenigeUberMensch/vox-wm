#ifndef XCB_DND_H_
#define XCB_DND_H_

#include "xcb_trl.h"




enum XCBDNDPROTOCOLS
{
    XDNDAware,
    XDNDProxy,
    XDNDActionAsk,
    XDNDActionCopy,
    XDNDActionLink,
    XDNDActionList,
    XDNDActionMove,
    XDNDActionPrivate,
    XDNDDrop,
    XDNDEnter,
    XDNDFinished,
    XDNDLeave,
    XDNDPosition,
    XDNDSelection,
    XDNDStatus,
    XDNDTypeList,
    XDNDLAST,
};



/* Fills cookie_return for DND atoms.
 */
void
XCBInitDNDAtomsCookie(
        XCBDisplay *display,
        XCBCookie *cookie_return
        );
void
XCBInitDNDAtomsReply(
        XCBDisplay *display,
        XCBCookie *cookies,
        XCBAtom *atom_return
        );

#endif
