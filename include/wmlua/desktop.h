#ifndef __WM__LUA__DESKTOP__H__
#define __WM__LUA__DESKTOP__H__

#include "wmlua/lua.h"


int l_desktop_current(lua_State *l);
int l_desktop_switch(lua_State *l);
int l_desktop_list(lua_State *l);

#endif
