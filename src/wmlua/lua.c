#include "util.h"
#include "config.h"

#include "wmlua/lua.h"
#include "wmlua/core.h"
#include "wmlua/desktop.h"
#include "wmlua/client.h"

lua_State *luastate = NULL;
lua_State *keybindThread = NULL;
static clock_t start;


/* luau stuff */

static void 
add_func(lua_State *L, const char *table, const char *name, lua_CFunction f)
{
    lua_getglobal(L, "wm");

    lua_getfield(L, -1, table);

    if (!lua_istable(L, -1)) 
    {
        lua_pop(L, 2);
        return;
    }

    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name);

    lua_pop(L, 2);
}

static void 
add_func_global(lua_State *L, const char *name, lua_CFunction f)
{
    lua_getglobal(L, "wm");

    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name);

    lua_pop(L, 1);
}

static void
hook(lua_State *L, lua_Debug *ar)
{
    (void)ar;

    clock_t tim = clock();

    if(tim - start > CLOCKS_PER_SEC / 60)
    {   luaL_error(L, "Max instruction count reached, ERROR: MAX_ALLOTED_TIME_EXCEEDED");
    }
}

/* funcs */

int
InitLua(void)
{
    if(!ASSERT(!luastate))
    {   return EXIT_SUCCESS;
    }

    luastate = luaL_newstate();

    if(unlikely(luastate == NULL))
    {   return EXIT_FAILURE;
    }

    luaL_openlibs(luastate);

    lua_newtable(luastate);

    lua_newtable(luastate);
    lua_setfield(luastate, -2, "client");

    lua_newtable(luastate);
    lua_setfield(luastate, -2, "workspace");

    lua_newtable(luastate);
    lua_setfield(luastate,  -2, "input");

    lua_newtable(luastate);
    lua_setfield(luastate, -2, "core");

    lua_setglobal(luastate, "wm");   // wm = table, pops it

    /* client */
    add_func(luastate, "client", "exists", l_client_exists);
    add_func(luastate, "client", "get", l_client_get);
    add_func(luastate, "client", "is_focused", l_client_is_focused);
    add_func(luastate, "client", "is_visible", l_client_is_visible);
    add_func(luastate, "client", "workspace", l_client_workspace);
    add_func(luastate, "client", "focus", l_client_focus);
    add_func(luastate, "client", "focus_next", l_client_focus_next);
    add_func(luastate, "client", "focus_prev", l_client_focus_prev);
    add_func(luastate, "client", "move", l_client_move);
    add_func(luastate, "client", "resize", l_client_resize);
    add_func(luastate, "client", "kill", l_client_kill);
    add_func(luastate, "client", "close", l_client_close);
    add_func(luastate, "client", "set_desktop", l_client_set_desktop);
    add_func(luastate, "client", "list", l_client_list);

    /* workspace */
    add_func(luastate, "workspace", "desktop_current", l_desktop_current);
    add_func(luastate, "workspace", "desktop_switch", l_desktop_switch);
    add_func(luastate, "workspace", "desktop_list", l_desktop_list);

    /* input */
    //add_func(luastate, "input", "focus", l_client_focus);

    /* wm core */
    add_func_global(luastate, "spawn", l_core_spawn);

    return EXIT_SUCCESS;
}

int
LuaRunKeybindThread(void)
{
    enum { MAX_INSTR = 1000 };

    if(!luastate)
    {   return EXIT_FAILURE;
    }

    const char *wmconfig = WMConfigGetPath(WMFileKeybinds);

    if(unlikely(!wmconfig))
    {   return EXIT_FAILURE;
    }

    /* start clock wen thread ready */
    start = clock();

    keybindThread = lua_newthread(luastate);

    if(!keybindThread)
    {   return EXIT_FAILURE;
    }

    lua_sethook(luastate, hook, LUA_MASKCOUNT, MAX_INSTR);

    int status;
    int nres = 0;

    status = luaL_loadfile(luastate, wmconfig);

    if(status != LUA_OK)
    {   
        DebugWarn("While loading keybinds.lua, encountered: %s", lua_tostring(keybindThread, -1));
        lua_pop(luastate, 1);
        return EXIT_FAILURE;
    }

    lua_xmove(luastate, keybindThread, 1);

    status = lua_resume(keybindThread, NULL, 0, &nres);

    (void)nres;

    if(status != LUA_OK && status != LUA_YIELD)
    {   
        DebugWarn("Runtime error: %s", lua_tostring(keybindThread, -1));
        lua_pop(keybindThread, 1);
        return EXIT_FAILURE;   
    }

    return EXIT_SUCCESS;
}

lua_State *
LuaGetKeybindThread(void)
{   return keybindThread;
}

void
DestroyLua(void)
{
    if(luastate)
    {   lua_close(luastate);
    }

    luastate = NULL;
    keybindThread = NULL;
}

