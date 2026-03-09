#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "usersettings.h"

SCSetting
UserSettingsDefault[] = 
{
    VOX_ADD_MEMBER_SETTING(MFact, SCTypeFLOAT, 0.55f, NULL)
    VOX_ADD_MEMBER_SETTING(GapRatio, SCTypeFLOAT, 0.95f, NULL)
    VOX_ADD_MEMBER_SETTING(MCount, SCTypeUSHORT, 1, NULL)
    VOX_ADD_MEMBER_SETTING(Snap, SCTypeUSHORT, 10, NULL)
    VOX_ADD_MEMBER_SETTING(RefreshRate, SCTypeUSHORT, 60, NULL)

    /* BOOL Types */
    VOX_ADD_MEMBER_SETTING(HoverFocus, SCTypeBOOL, false, NULL)
    VOX_ADD_MEMBER_SETTING(UseDecorations, SCTypeBOOL, false, NULL)
    VOX_ADD_MEMBER_SETTING(UseClientSideDecorations, SCTypeBOOL, true, NULL)
    VOX_ADD_MEMBER_SETTING(PreferClientSideDecorations, SCTypeBOOL, true, NULL)


    /* bar data */
    VOX_ADD_MEMBER_SETTING(BarLX, SCTypeFLOAT, 0.0f, NULL)    /*   lx    */
    VOX_ADD_MEMBER_SETTING(BarLY, SCTypeFLOAT, 0.0f, NULL)    /*   ly    */
    VOX_ADD_MEMBER_SETTING(BarLW, SCTypeFLOAT, 0.15f, NULL)   /*   lw    */
    VOX_ADD_MEMBER_SETTING(BarLH, SCTypeFLOAT, 1.0f, NULL)    /*   lh    */

    VOX_ADD_MEMBER_SETTING(BarRX, SCTypeFLOAT, 0.85f, NULL)   /* rx - rw */
    VOX_ADD_MEMBER_SETTING(BarRY, SCTypeFLOAT, 0.0f, NULL)    /*   ry    */
    VOX_ADD_MEMBER_SETTING(BarRW, SCTypeFLOAT, 0.15f, NULL)   /*   rw    */
    VOX_ADD_MEMBER_SETTING(BarRH, SCTypeFLOAT, 1.0f, NULL)    /*   rh    */

    VOX_ADD_MEMBER_SETTING(BarTX, SCTypeFLOAT, 0.0f, NULL)    /*   tx    */
    VOX_ADD_MEMBER_SETTING(BarTY, SCTypeFLOAT, 0.0f, NULL)    /*   ty    */
    VOX_ADD_MEMBER_SETTING(BarTW, SCTypeFLOAT, 1.0f, NULL)    /*   tw    */
    VOX_ADD_MEMBER_SETTING(BarTH, SCTypeFLOAT, 0.15f, NULL)   /*   th    */

    VOX_ADD_MEMBER_SETTING(BarBX, SCTypeFLOAT, 0.0f, NULL)    /*   bx    */
    VOX_ADD_MEMBER_SETTING(BarBY, SCTypeFLOAT, 0.85f, NULL)   /* bx - bh */
    VOX_ADD_MEMBER_SETTING(BarBW, SCTypeFLOAT, 1.0f, NULL)    /*   bw    */
    VOX_ADD_MEMBER_SETTING(BarBH, SCTypeFLOAT, 0.15f, NULL)   /*   bh    */
};

