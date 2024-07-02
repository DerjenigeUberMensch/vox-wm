
#include "xcb_trl.h"

/* macros */
#ifndef XEMBED_EMBEDDED_NOTIFY
#define XEMBED_EMBEDDED_NOTIFY      0
#endif

#ifndef XEMBED_WINDOW_ACTIVATE
#define XEMBED_WINDOW_ACTIVATE      1
#endif

#ifndef XEMBED_WINDOW_DEACTIVATE
#define XEMBED_WINDOW_DEACTIVATE    2
#endif

#ifndef XEMBED_REQUEST_FOCUS
#define XEMBED_REQUEST_FOCUS        3
#endif

#ifndef XEMBED_FOCUS_IN
#define XEMBED_FOCUS_IN             4
#endif

#ifndef XEMBED_FOCUS_OUT
#define XEMBED_FOCUS_OUT            5
#endif

#ifndef XEMBED_FOCUS_NEXT
#define XEMBED_FOCUS_NEXT           6
#endif

#ifndef XEMBED_FOCUS_PREV
#define XEMBED_FOCUS_PREV           7
#endif

#ifndef XEMBED_MODALITY_ON
#define XEMBED_MODALITY_ON          10
#endif

#ifndef XEMBED_MODALITY_OFF
#define XEMBED_MODALITY_OFF         11
#endif

#ifndef XEMBED_REGISTER_ACCELERATOR
#define XEMBED_REGISTER_ACCELERATOR 12
#endif

#ifndef XEMBED_UNREGISTER_ACCELERATOR
#define XEMBED_UNREGISTER_ACCELERATOR 13
#endif

#ifndef XEMBED_ACTIVATE_ACCELERATOR
#define XEMBED_ACTIVATE_ACCELERATOR 14
#endif



#ifndef XEMBED_FOCUS_CURRENT
#define XEMBED_FOCUS_CURRENT 0
#endif

#ifndef XEMBED_FOCUS_FIRST
#define XEMBED_FOCUS_FIRST 1
#endif

#ifndef XEMBED_FOCUS_LAST
#define XEMBED_FOCUS_LAST 2
#endif



#ifndef XEMBED_MODIFIER_SHIFT
#define XEMBED_MODIFIER_SHIFT    (1 << 0)
#endif

#ifndef XEMBED_MODIFIER_CONTROL
#define XEMBED_MODIFIER_CONTROL  (1 << 1)
#endif

#ifndef XEMBED_MODIFIER_ALT
#define XEMBED_MODIFIER_ALT      (1 << 2)
#endif

#ifndef XEMBED_MODIFIER_SUPER
#define XEMBED_MODIFIER_SUPER    (1 << 3)
#endif

#ifndef XEMBED_MODIFIER_HYPER
#define XEMBED_MODIFIER_HYPER    (1 << 4)
#endif


#ifndef XEMBED_ACCELERATOR_OVERLOADED 
#define XEMBED_ACCELERATOR_OVERLOADED (1 << 0)
#endif


enum XEMBEDATOMS
{
    Xembed,
    XEMBEDLAST,
};



void
XCBInitXembedAtomsCookie(
        XCBDisplay *display,
        XCBCookie *xembed_cookie_return
        );

void
XCBInitXembedAtomsReply(
        XCBDisplay *display,
        XCBCookie *cookies,
        XCBAtom *atom_return
        );



