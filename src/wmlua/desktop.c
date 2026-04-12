#include "wmlua/lua.h"
#include "main.h"
#include "toggle.h"

#define VERIFY_DESKTOP_ID(id) if(id < 0 || id > UINT32_MAX) { return luaL_error(l, "invalid client id"); }

extern WM _wm;

int
l_desktop_current(lua_State *l)
{
    int pushedVars = 0;
    u32 desktopid = 0;

    LOCK_WM();

    desktopid = _wm.selmon->desksel->num;

    UNLOCK_WM();

    lua_pushinteger(l, desktopid);
    ++pushedVars;

    return pushedVars;
}

int
l_desktop_switch(lua_State *l)
{
    lua_Integer desktopid = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_DESKTOP_ID(desktopid);

    u32 desktopidreal = (u32)desktopid;
    u32 startindesktopid = 0;
    bool failed = false;
    Arg arg;

    LOCK_WM();

    startindesktopid = _wm.selmon->desksel->num;

    arg.ul = desktopidreal;

    ToggleDesktop(&arg);

    failed = startindesktopid == _wm.selmon->desksel->num && startindesktopid != desktopidreal;

    UNLOCK_WM();

    lua_pushboolean(l, !failed);
    ++pushedVars;

    return pushedVars;
}

int
l_desktop_list(lua_State *l)
{
    int pushedVars = 0;

    lua_newtable(l);
    ++pushedVars;

    /* lua table are indexed at 1 for some reason... */
    garray_i luatablei = 1;
    Desktop *desk;

    LOCK_WM();

    for(desk = _wm.mons->desktops; desk; desk = nextdesktop(desk))
    {   
        lua_pushinteger(l, desk->num);
        lua_rawseti(l, -2, ++luatablei);
    }

    UNLOCK_WM();

    return pushedVars;
}

