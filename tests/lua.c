#include <lua.h>
#include <lauxlib.h>

int main() {
    lua_State *L = luaL_newstate();
    if (!L) return 1;
    lua_close(L);
    return 0;
}
