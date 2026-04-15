#ifndef __VOX__WM__LUA__H__
#define __VOX__WM__LUA__H__

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"


int InitLua(void);
int LuaRunKeybindThread(void);
void DestroyLua(void);


#endif
