#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
#include "config.h"
#include "file_util.h"
#include "threading.h"

#include "wmlua/lua.h"
#include "wmlua/core.h"
#include "wmlua/desktop.h"
#include "wmlua/client.h"
#include "wmlua/input.h"

lua_State *luastate = NULL;
static bool threadRunning = false;
static u32 luaInstrCount = 0;
static bool isHalt = false;
static bool isDead = false;

pthread_mutex_t lua_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t halt_cond = PTHREAD_COND_INITIALIZER;

int
TryLockLua(void)
{   return pthread_mutex_trylock(&lua_mutex);
}

int 
LockLua(void)
{   return pthread_mutex_lock(&lua_mutex);
}

int
UnlockLua(void)
{   return pthread_mutex_unlock(&lua_mutex);
}


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

/* funcs */

int
InitLua(void)
{
    if(unlikely(pthread_mutex_init(&lua_mutex, NULL) != 0))
    {   
        DebugWarn("Failed to initialize Lua mutex");
        return EXIT_FAILURE;
    }

    if(unlikely(pthread_cond_init(&halt_cond, NULL) != 0))
    {
        DebugWarn("Failed to initialize Lua condition");
        return EXIT_FAILURE;
    }

    LockLua();

    if(!ASSERT(!luastate))
    {   
        UnlockLua();
        return EXIT_SUCCESS;
    }

    luastate = luaL_newstate();

    if(unlikely(luastate == NULL))
    {   
        UnlockLua();
        return EXIT_FAILURE;
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
    add_func(luastate, "input", "bind", l_input_bind);

    /* wm core */
    add_func_global(luastate, "spawn", l_core_spawn);

    const char *wmconfig = WMConfigGetPath(WMFileLua);

    if(wmconfig)
    {
        if(!FFFileExists((char *)wmconfig))
        {   
            FFCreateFile((char *)wmconfig);
            DebugLog("Lua config file not found, creating default at %s", wmconfig);
        }
    }
    
    enum { FILE_PATH_LEN = 1024 };
    enum { MAX_PATH_LEN = FFSysGetConfigPathLengthMAX + FILE_PATH_LEN };

    char buff[MAX_PATH_LEN];

    snprintf(buff, MAX_PATH_LEN, 
        "package.path = '%s' .. package.path",
        WMConfigGetPath(WMFileFolder)
    );

    luaL_dostring(luastate, "package.path = './scripts/?.lua;./scripts/?/init.lua;' .. package.path");

    UnlockLua();

    return EXIT_SUCCESS;
}

bool 
LuaIsThreadRunning(void)
{
    bool running;

    LockLua();
    running = threadRunning;
    UnlockLua();

    return running;
}

bool
LuaIsDead(void)
{
    bool dead;

    LockLua();
    dead = isDead;
    UnlockLua();

    return dead;
}

static void
LUA_INSTR_MAX_REACHED_HOOK(lua_State *L, lua_Debug *dgb)
{
    luaInstrCount += LuaInstrLimitIncrement;

    /* we are erroring out so if a cache miss happens here it doesnt matter */
    if(unlikely(luaInstrCount >= LuaInstrLimitMax))
    {   luaL_error(L, "Lua instruction limit reached (your banned)");
    }
}

static void
LUA_HALT_HOOK(lua_State *L, lua_Debug *dgb)
{
    if(isHalt)
    {   lua_yield(L, 0);
    }
}

static void
LUA_HOOK_HANDLER(lua_State *L, lua_Debug *dgb)
{
    /* must already be locked or we are in a UNSAFE state */
    ASSERT(TryLockLua() != 0);

    LUA_INSTR_MAX_REACHED_HOOK(L, dgb);
    LUA_HALT_HOOK(L, dgb);
}

static void
LUA_RUN_THREAD_IMPL(Generic *unused)
{
    LockLua();

    if(!luastate)
    {   goto UNLOCK;
    }

    const char *wmconfig = WMConfigGetPath(WMFileLua);

    if(unlikely(!wmconfig))
    {   goto UNLOCK;
    }

    lua_State *luaThread= NULL;

    luaThread = lua_newthread(luastate);

    if(!luaThread)
    {   goto UNLOCK;
    }

    int status;
    int nres = 0;

    status = luaL_loadfile(luastate, wmconfig);

    if(status != LUA_OK)
    {   
        char *file = strdup(wmconfig);
        char *filename = file;

        if(filename)
        {   basename(filename);
        }

        DebugWarn("While loading %s, encountered: %s", filename ? filename : "Not Found", lua_tostring(luaThread, -1));

        free(file);

        lua_pop(luastate, 1);

        goto UNLOCK;
    }

    lua_xmove(luastate, luaThread, 1);

    threadRunning = true;

    lua_sethook(luaThread, LUA_HOOK_HANDLER, LUA_MASKCOUNT, LuaInstrLimitIncrement);

    do
    {
        status = lua_resume(luaThread, NULL, 0, &nres);

        (void)nres;

        luaInstrCount = 0;

        while(isHalt)
        {   pthread_cond_wait(&halt_cond, &lua_mutex);
        }

    } while(status == LUA_YIELD && !isDead);

    if (status != LUA_OK) 
    {
        DebugWarn("Runtime error: %s", lua_tostring(luaThread, -1));
        lua_pop(luaThread, 1);
    }

    /* unset hook */
    lua_sethook(luaThread, NULL, 0, 0);

UNLOCK:
    threadRunning = false;
    UnlockLua();
}

int
LuaRunThread(void)
{
    bool running;
    int status;

    LockLua();

    running = threadRunning;

    if(!running)
    {
        if(ThreadingUsesThreads())
        {   status = ThreadingAddWork(LUA_RUN_THREAD_IMPL, NULL, NULL);
        }
    }

    UnlockLua();

    if(running)
    {   return EXIT_SUCCESS;
    }

    if(status == EXIT_FAILURE)
    {   
        DebugWarn("Failed to start Lua keybind thread");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
LuaRequestHalt(void)
{
    LockLua();

    isHalt = true;

    UnlockLua();

    return EXIT_SUCCESS;
}

int
LuaResumeAfterHalt(void)
{
    LockLua();

    isHalt = false;

    pthread_cond_signal(&halt_cond);

    UnlockLua();

    return EXIT_SUCCESS;
}

void
DestroyLua(void)
{
    LuaRequestHalt();

    LockLua();

    isDead = true;

    UnlockLua();

    LuaResumeAfterHalt();

    while(LuaIsThreadRunning());

    LockLua();

    if(luastate)
    {   lua_close(luastate);
    }

    luastate = NULL;
    threadRunning = false;

    UnlockLua();

    pthread_mutex_destroy(&lua_mutex);
    pthread_cond_destroy(&halt_cond);
}