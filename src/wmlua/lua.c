#include "wmlua/lua.h"
#include "util.h"

lua_State *luastate = NULL;


/* luau stuff */

void 
register_func(lua_State *L, const char *root, const char *sub, const char *name, lua_CFunction f)
{
    lua_getglobal(L, root);

    if (!lua_istable(L, -1)) 
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, root);
    }

    lua_getfield(L, -1, sub);

    if (!lua_istable(L, -1)) 
    {
        lua_pop(L, 1);
        lua_newtable(L);

        lua_pushvalue(L, -1);
        lua_setfield(L, -3, sub);
    }

    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name);

    lua_pop(L, 2);
}


/* funcs */

int
InitLua(void)
{
    luastate = luaL_newstate();

    if(unlikely(luastate == NULL))
    {   return EXIT_FAILURE;
    }

    luaL_openlibs(luastate);

    //register_func(luastate, "wm", "input", "bind", l_bind);


    return EXIT_SUCCESS;
}

void
DestroyLua(void)
{
    if(luastate)
    {   lua_close(luastate);
    }

    luastate = NULL;
}

