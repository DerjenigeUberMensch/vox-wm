#include "xcb_trl.h"




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
