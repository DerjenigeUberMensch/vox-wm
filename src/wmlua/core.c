#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>

#include "main.h"

#include "wmlua/lua.h"
#include "wmlua/util.h"

#include "wmlua/core.h"

extern WM _wm;

int
l_core_spawn(lua_State *l)
{
    const char *cmd = luaL_checkstring(l, 1);
    int pushedVars = 0;

    if(!cmd || cmd[0] == '\0')
    {   
        lua_pushboolean(l, 0);
        ++pushedVars;
        return pushedVars;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        lua_pushboolean(l, 0);
        ++pushedVars;
        return pushedVars;
    }

    if (!pid)
    {
        // child
        setsid(); // detach from WM

        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);

        _exit(EXIT_FAILURE);
    }

    lua_pushboolean(l, 1);
    ++pushedVars;

    return pushedVars;
}

