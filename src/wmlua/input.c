#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/keysym.h>
#include <X11/XF86keysym.h> 

#include "wmlua/input.h"
#include "main.h"
#include "keybinds.h"


extern WM _wm;

int 
strcmp_lower(const char *a, const char *b)
{
    char ca;
    char cb;

    while (*a && *b) 
    {
        ca = tolower((unsigned char)*a);
        cb = tolower((unsigned char)*b);

        if (ca != cb) 
        {   return ca - cb;
        }

        ++a;
        ++b;
    }

    return *a - *b;
}

int 
l_input_bind(lua_State *l)
{
    const char *combo = lua_tostring(l, 1);

    if(!combo || combo[0] == '\0')
    {   return luaL_error(l, "invalid bind");
    }

    enum { BUFF_SIZE = 1024 };

    char buff[BUFF_SIZE];

    memset(buff, 0, BUFF_SIZE);

    strncpy(buff, combo, (BUFF_SIZE - 1) * sizeof(char));

    u32 mask = 0;
    u32 i = 0;
    XCBKeysym keycode = 0;

    char *saveptr;
    char *token = strtok_r(buff, "+", &saveptr);

    while (token)
    {
        bool found = false;

        // modifier check
        for (i = 0; i < LENGTH(mods_table); ++i)
        {
            if (!strcmp_lower(token, mods_table[i].name))
            {
                mask |= mods_table[i].keycode;
                found = true;
                break;
            }
        }

        // keycode check
        if (!found)
        {
            for (i = 0; i < LENGTH(keycode_table); ++i)
            {
                if (!strcmp_lower(token, keycode_table[i].name))
                {
                    keycode = keycode_table[i].keycode;
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            DebugWarn("no found");
            break;
        }

        token = strtok_r(NULL, "+", &saveptr);
    }

    return 0;
}
