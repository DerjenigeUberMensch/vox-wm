#ifndef __XCB__TRL__GTK__H__
#define __XCB__TRL__GTK__H__

#include "xcb_trl.h"


#ifdef __cplusplus
extern "C" {
#endif



enum XCBGTKPROTOCOLS
{
    GTKEdgeConstraints,
    GTKShowWindowMenu,
    GTKWorkareas,
    GTKFrameExtents,
    GTKThemeVariant,
    /* gdk */
    GTKSelection,
    GTKClipboardSaveTargets,
    GTKVisuals,
    /* Not sure */
    GTKNetStartupID,



    GTKLAST,
};



/* Fills wm_atom_return and net_atom_return with atoms know for GTK
 */
void
XCBInitGTKAtomsCookie(
        XCBDisplay *display,
        XCBCookie *gtk_cookie_return
        );

void
XCBInitGTKAtomsReply(
        XCBDisplay *display,
        XCBCookie *gtk_cookies,
        XCBAtom *gtk_atom_return
        );

#ifdef __cplusplus
}
#endif

#endif
