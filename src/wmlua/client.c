#include "wmlua/lua.h"
#include "wmlua/client.h"
#include "main.h"

extern WM _wm;

int
l_client_exists(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    Client *c;
    XCBWindow realid = (XCBWindow)id;

    LOCK_WM();

    c = wintoclient(realid);

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_get(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    Client *c;

    XCBWindow realid = (XCBWindow)id;

    /* lua table are indexed at 1 for some reason... */
    Client cpy;

    LOCK_WM();

    c = wintoclient(realid);

    if(c)
    {   cpy = *c;
    }

    UNLOCK_WM();

    if(c)
    {
        lua_newtable(l);
        ++pushedVars;

        lua_pushinteger(l, cpy.win);
        lua_setfield(l, -2, "id");

        lua_pushinteger(l, cpy.x);
        lua_setfield(l, -2, "x");

        lua_pushinteger(l, cpy.y);
        lua_setfield(l, -2, "y");

        lua_pushinteger(l, cpy.w);
        lua_setfield(l, -2, "w");

        lua_pushinteger(l, cpy.h);
        lua_setfield(l, -2, "h");
    }
    else
    {
        lua_pushnil(l);
        ++pushedVars;
    }

    return pushedVars;
}

int
l_client_is_focused(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    XCBWindow realid = (XCBWindow)id;

    Client *c;
    bool focused = false;

    LOCK_WM();

    c = wintoclient(realid);

    if(c)
    {   focused = ISFOCUSED(c);
    }

    UNLOCK_WM();

    if(c)
    {   lua_pushboolean(l, focused);
    }
    else
    {   lua_pushnil(l);
    }

    ++pushedVars;

    return pushedVars;
}

int
l_client_is_visible(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    XCBWindow realid = (XCBWindow)id;

    Client *c;
    bool visible = false;

    LOCK_WM();

    c = wintoclient(realid);
    
    if(c)
    {   visible = ISVISIBLE(c);
    }

    UNLOCK_WM();

    if(c)
    {   lua_pushboolean(l, visible);
    }
    else
    {   lua_pushnil(l);
    }

    ++pushedVars;

    return pushedVars;
}

int
l_client_workspace(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    XCBWindow realid = (XCBWindow)id;

    Client *c;
    i16 desknum;

    LOCK_WM();

    c = wintoclient(realid);

    if(c)
    {   desknum = c->desktop->num;
    }

    UNLOCK_WM();


    if(c)
    {   lua_pushinteger(l, desknum);
    }
    else
    {   lua_pushnil(l);
    }
    ++pushedVars;

    return pushedVars;
}


int
l_client_focus(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    XCBWindow realid = (XCBWindow)id;

    Client *c;

    LOCK_WM();

    c = wintoclient(realid);

    if(c)
    {   focus(c);
    }

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_focus_next(lua_State *l)
{
    int pushedVars = 0;

    Client *c;
    Client *next = NULL;

    LOCK_WM();

    c = _wm.selmon->desksel->sel;

    if(!c)
    {   c = _wm.selmon->desksel->focus;
    }

    if(c)
    {
        next = nextfocus(c);

        if(!next)
        {   next = startfocus(c->desktop);
        }

        if(next)
        {   focus(next);
        }
    }

    UNLOCK_WM();

    lua_pushboolean(l, next != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_focus_prev(lua_State *l)
{
    int pushedVars = 0;

    Client *c;
    Client *prev = NULL;

    LOCK_WM();

    c = _wm.selmon->desksel->sel;

    if(!c)
    {   c = _wm.selmon->desksel->focus;
    }

    if(c)
    {
        prev = prevfocus(c);

        if(!prev)
        {   prev = lastfocus(c->desktop);
        }

        if(prev)
        {   focus(prev);
        }
    }

    UNLOCK_WM();

    lua_pushboolean(l, prev != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_move(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer x = luaL_checkinteger(l, 2);
    lua_Integer y = luaL_checkinteger(l, 3);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    x = CLAMP(x, INT16_MIN, INT16_MAX);
    y = CLAMP(y, INT16_MIN, INT16_MAX);

    i32 realx = (i32)x;
    i32 realy = (i32)y;
    XCBWindow realid = (XCBWindow)id;

    Client *c;

    LOCK_WM();

    c = wintoclient(realid);

    /* TODO: Let users move off screen if they want to */
    if(c)
    {   resizemove(c, realx, realy, 0);
    }

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_resize(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer w = luaL_checkinteger(l, 2);
    lua_Integer h = luaL_checkinteger(l, 3);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    w = CLAMP(w, 1, UINT16_MAX);
    h = CLAMP(h, 1, UINT16_MAX);

    i32 realw = (i32)w;
    i32 realh = (i32)h;
    XCBWindow realid = (XCBWindow)id;

    Client *c;

    LOCK_WM();

    c = wintoclient(realid);

    /* TODO: Let users move off screen if they want to */
    if(c)
    {   resize(c, c->x, c->y, realw, realh, 0);
    }

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_kill(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    Client *c;
    XCBWindow realid = (XCBWindow)id;

    LOCK_WM();

    c = wintoclient(realid);

    if(c)
    {   killclient(c, Safedestroy);
    }

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_close(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);

    Client *c;
    XCBWindow realid = (XCBWindow)id;

    LOCK_WM();

    c = wintoclient(realid);

    if(c)
    {   killclient(c, Graceful);
    }

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL);
    ++pushedVars;

    return pushedVars;
}

int
l_client_set_desktop(lua_State *l)
{
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer deskid = luaL_checkinteger(l, 2);

    int pushedVars = 0;

    VERIFY_CLIENT_ID(id);
    VERIFY_DESKTOP_ID(deskid);

    Client *c;
    Desktop *desk = NULL;
    XCBWindow realid = (XCBWindow)id;
    u32 realdeskid = (u32)deskid;

    LOCK_WM();

    c = wintoclient(realid);
    
    if(c)
    {   
        /* find the desktop */
        for(desk = _wm.selmon->desktops; desk; desk = nextdesktop(desk))
        {
            if(desk->num == realdeskid)
            {   break;
            }
        }

        if(desk)
        {   setclientdesktop(c, desk);
        }
    }

    UNLOCK_WM();

    lua_pushboolean(l, c != NULL && desk != NULL);
    ++pushedVars;

    return pushedVars;
}


int
l_client_list(lua_State *l)
{
    int pushedVars = 0;

    lua_newtable(l);
    ++pushedVars;

    /* lua table are indexed at 1 for some reason... */
    garray_i luatablei = 1;
    garray_i i;

    LOCK_WM();

    for(i = GArrayStart(&_wm.clients); i < GArrayEnd(&_wm.clients); ++i)
    {   
        XCBWindow *win = GArrayAt(&_wm.clients, i);

        if(likely(win))
        {   
            lua_pushinteger(l, (lua_Integer)*win);
            lua_rawseti(l, -2, ++luatablei);
        }
    }

    UNLOCK_WM();

    return pushedVars;
}

