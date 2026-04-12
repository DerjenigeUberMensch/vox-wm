#ifndef __WM__LUA__CLIENT__H__
#define __WM__LUA__CLIENT__H__


#include "wmlua/lua.h"
#include "wmlua/util.h"
#include "main.h"

extern WM _wm;

int l_client_exists(lua_State *l);
int l_client_get(lua_State *l);
int l_client_is_focused(lua_State *l);
int l_client_is_visible(lua_State *l);
int l_client_workspace(lua_State *l);
int l_client_focus(lua_State *l);
int l_client_focus_next(lua_State *l);
int l_client_focus_prev(lua_State *l);
int l_client_move(lua_State *l);
int l_client_resize(lua_State *l);
int l_client_kill(lua_State *l);
int l_client_close(lua_State *l);
int l_client_set_desktop(lua_State *l);
int l_client_list(lua_State *l);

#endif
