#include "wmlua/lua.h"
#include "main.h"

#define VERIFY_CLIENT_ID(id) if(id < 0 || id > UINT32_MAX) { return luaL_error(l, "invalid client id"); }

extern WM _wm;

int
l_core_spawn(lua_State *l)
{
    int pushedVars = 0;

    return pushedVars;
}

