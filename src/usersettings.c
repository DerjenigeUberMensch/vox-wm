#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "main.h"
#include "desktop.h"
#include "usersettings.h"



#define USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current) if(!memcmp(&prev, &current, sizeof(current))) { return; }


extern WM _wm;


static void UpdateDesktop(void);
static void UpdateSkip(Generic prev, Generic current);
static void UpdateMFact(Generic prev, Generic current);
static void UpdateGapRatio(Generic prev, Generic current);
static void UpdateMCount(Generic prev, Generic current);

static void UpdateUseDecorations(Generic prev, Generic current);
static void UpdateUseClientSideDecorations(Generic prev, Generic current);
static void UpdatePreferClientSideDecorations(Generic prev, Generic current);
static void UpdateBarXYWHChange(Generic prev, Generic current);

SCSetting
UserSettingsDefault[] = 
{
    VOX_ADD_MEMBER_SETTING(MFact, SCTypeFLOAT, 0.55f, UpdateMFact)
    VOX_ADD_MEMBER_SETTING(GapRatio, SCTypeFLOAT, 0.95f, UpdateGapRatio)
    VOX_ADD_MEMBER_SETTING(MCount, SCTypeUSHORT, 1, UpdateMCount)
    VOX_ADD_MEMBER_SETTING(Snap, SCTypeUSHORT, 10, UpdateSkip)
    VOX_ADD_MEMBER_SETTING(RefreshRate, SCTypeUSHORT, 60, UpdateSkip)
    VOX_ADD_MEMBER_SETTING(SmartResizing, SCTypeBOOL, true, UpdateSkip)
    VOX_ADD_MEMBER_SETTING(CenteringBias, SCTypeFLOAT, .0125, UpdateSkip)


    /* BOOL Types */
    VOX_ADD_MEMBER_SETTING(HoverFocus, SCTypeBOOL, false, UpdateSkip)
    VOX_ADD_MEMBER_SETTING(UseDecorations, SCTypeBOOL, false, UpdateUseDecorations)
    VOX_ADD_MEMBER_SETTING(UseClientSideDecorations, SCTypeBOOL, true, UpdateUseClientSideDecorations)
    VOX_ADD_MEMBER_SETTING(PreferClientSideDecorations, SCTypeBOOL, true, UpdatePreferClientSideDecorations)

    /* legacy */

    VOX_ADD_MEMBER_SETTING(UseLegacyFloatingSystem, SCTypeBOOL, false, UpdateSkip)


    /* bar data */
    VOX_ADD_MEMBER_SETTING(BarLX, SCTypeFLOAT, 0.0f, UpdateBarXYWHChange)    /*   lx    */
    VOX_ADD_MEMBER_SETTING(BarLY, SCTypeFLOAT, 0.0f, UpdateBarXYWHChange)    /*   ly    */
    VOX_ADD_MEMBER_SETTING(BarLW, SCTypeFLOAT, 0.15f, UpdateBarXYWHChange)   /*   lw    */
    VOX_ADD_MEMBER_SETTING(BarLH, SCTypeFLOAT, 1.0f, UpdateBarXYWHChange)    /*   lh    */

    VOX_ADD_MEMBER_SETTING(BarRX, SCTypeFLOAT, 0.85f, UpdateBarXYWHChange)   /* rx - rw */
    VOX_ADD_MEMBER_SETTING(BarRY, SCTypeFLOAT, 0.0f, UpdateBarXYWHChange)    /*   ry    */
    VOX_ADD_MEMBER_SETTING(BarRW, SCTypeFLOAT, 0.15f, UpdateBarXYWHChange)   /*   rw    */
    VOX_ADD_MEMBER_SETTING(BarRH, SCTypeFLOAT, 1.0f, UpdateBarXYWHChange)    /*   rh    */

    VOX_ADD_MEMBER_SETTING(BarTX, SCTypeFLOAT, 0.0f, UpdateBarXYWHChange)    /*   tx    */
    VOX_ADD_MEMBER_SETTING(BarTY, SCTypeFLOAT, 0.0f, UpdateBarXYWHChange)    /*   ty    */
    VOX_ADD_MEMBER_SETTING(BarTW, SCTypeFLOAT, 1.0f, UpdateBarXYWHChange)    /*   tw    */
    VOX_ADD_MEMBER_SETTING(BarTH, SCTypeFLOAT, 0.15f, UpdateBarXYWHChange)   /*   th    */

    VOX_ADD_MEMBER_SETTING(BarBX, SCTypeFLOAT, 0.0f, UpdateBarXYWHChange)    /*   bx    */
    VOX_ADD_MEMBER_SETTING(BarBY, SCTypeFLOAT, 0.85f, UpdateBarXYWHChange)   /* bx - bh */
    VOX_ADD_MEMBER_SETTING(BarBW, SCTypeFLOAT, 1.0f, UpdateBarXYWHChange)    /*   bw    */
    VOX_ADD_MEMBER_SETTING(BarBH, SCTypeFLOAT, 0.15f, UpdateBarXYWHChange)   /*   bh    */
};

static void 
UpdateDesktop(void)
{
    LOCK_WM();

    /* due to USLoad being set to before WM is intialized fully this may be null. */

    if(_wm.selmon && _wm.selmon->desksel)
    {   arrange(_wm.selmon->desksel);
    }

    /* no checks for dpy as its guranteed to exist until cleanup() is called */
    XCBFlush(_wm.dpy);

    UNLOCK_WM();
}

static void
UpdateSkip(Generic prev, Generic current)
{   
    (void)prev;
    (void)current;
    return;
}

static void 
UpdateMFact(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);

    UpdateDesktop();
}

static void 
UpdateGapRatio(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);

    UpdateDesktop();
}

static void 
UpdateMCount(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);

    UpdateDesktop();
}

static void 
UpdateUseDecorations(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);
    UpdateDesktop();
}

static void 
UpdateUseClientSideDecorations(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);
}

static void 
UpdatePreferClientSideDecorations(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);
}

static void 
UpdateBarXYWHChange(Generic prev, Generic current)
{
    USER_SETTINGS_RETURN_IF_NO_CHANGE(prev, current);

    UpdateDesktop();
}
