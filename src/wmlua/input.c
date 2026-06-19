#include <stdlib.h>
#include <string.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XF86keysym.h> 

#include "wmlua/input.h"
#include "GArray/garray.h"
#include "VXExtDebug/vxextdebug.h"
#include "XCB-TRL/xcb_trl_types.h"
#include "main.h"
#include "keybinds.h"
#include "util.h"
#include "wmlua/lua.h"

extern WM _wm;

void
LuaKeybindInputHandler(const Key *keybind_reference, const Arg *arg)
{
    if(!arg || !arg->v)
    {   
        DebugError("Invalid keybind reference in LuaKeybindInputHandler\n");
        return;
    }

    int ref = keybind_reference->lua_ref;
    lua_State *l = arg->v;

    if(LuaIsDead())
    {   
        DebugWarn("Lua thread is not running");
        return;
    }

    extern int LockLua(void);
    extern int UnlockLua(void);

    LuaRequestHalt();

    LockLua();

    lua_rawgeti(l, LUA_REGISTRYINDEX, ref);

    if(lua_pcall(l, 0, 0, 0) != LUA_OK)
    {
        DebugError("Error calling keybind function: %s\n", lua_tostring(l, -1));
        lua_pop(l, 1);
    }

    UnlockLua();

    LuaResumeAfterHalt();
}

int 
l_input_bind(lua_State *l)
{
    const char *combo = lua_tostring(l, 1);

    if(!combo || combo[0] == '\0')
    {   return luaL_error(l, "invalid bind");
    }

    if(!lua_isfunction(l, 2))
    {   return luaL_error(l, "bind must be a function");
    }

    enum { BUFF_SIZE = 1024 };

    char buff[BUFF_SIZE];

    memset(buff, 0, BUFF_SIZE);

    strncpy(buff, combo, (BUFF_SIZE - 1) * sizeof(char));

    enum { KEYSYM_SIZE = sizeof(XCBKeysym) };
    GArray keysyms = GARRAY_STATIC_INITIALIZER(KEYSYM_SIZE);

    u16 modmask = 0;
    int status;

    char *saveptr;
    char *token = strtok_r(buff, "+", &saveptr);

    while (token)
    {
        XCBKeysym recognizedsim = 0;

        recognizedsim = WMKeybindKeysymFromString(token);

        if(recognizedsim != NoSymbol)
        {   
            status = GArrayPushBack(&keysyms, &recognizedsim);

            if(status == EXIT_FAILURE)
            {   
                GArrayWipe(&keysyms);
                return luaL_error(l, "failed to add keysym to array (OutOfMemory)");
            }
        }
        else
        {   modmask |= WMKeybindModifierFromString(token);
        }

        token = strtok_r(NULL, "+", &saveptr);
    }

    void *arr;
    size_t length;

    GArrayGetArray(&keysyms, &arr, &length, NULL, NULL);

    Key keybind;
    int ref;
    Arg arg;

    /* push the function */
    lua_pushvalue(l, 2);

    ref = luaL_ref(l, LUA_REGISTRYINDEX);

    arg = (Arg){ .v = l };

    if(modmask != 0 || length > 0)
    {   status = WMKeybindCreate(&keybind, modmask, arr, length, LuaKeybindInputHandler, arg, ref, true);
    }

    GArrayWipe(&keysyms);

    if(status == EXIT_FAILURE)
    {   
        /* free memory */
        luaL_unref(l, LUA_REGISTRYINDEX, ref); 
        return luaL_error(l, "failed to create keybind");
    }

    if(status == EXIT_SUCCESS)
    {   WMKeybindAdd(&keybind);
    }

    status = WMKeybindRefresh();

    (void)ASSERT(status == EXIT_SUCCESS);

    return 0;
}

