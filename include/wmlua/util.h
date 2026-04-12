#ifndef __VOX__WM__LUA__UTILS__H__
#define __VOX__WM__LUA__UTILS__H__


#ifndef VERIFY_CLIENT_ID
    #define VERIFY_CLIENT_ID(id) if(id < 0 || id > UINT32_MAX) { return luaL_error(l, "invalid client id"); }
#endif

#ifndef VERIFY_DESKTOP_ID
    #define VERIFY_DESKTOP_ID(id) if(id < 0 || id > UINT32_MAX) { return luaL_error(l, "invalid client id"); }
#endif

#endif
